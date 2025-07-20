package main

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"log"
	"net"
	"os"
	"sync"
	"time"
)

// Configuration structure
type Config struct {
	ServerPort   int    `json:"server_port"`
	MaxPlayers   int    `json:"max_players"`
	ServerName   string `json:"server_name"`
	LogLevel     string `json:"log_level"`
	TickRate     int    `json:"tick_rate"`
}

// PacketType represents the different types of network packets
type PacketType uint8

const (
	PacketConnect PacketType = iota
	PacketCreateShip
	PacketDeleteShip
	PacketCreateStar
	PacketShipUpdate
	PacketConnectAcknowledge
	PacketChatMessage
	PacketPlayerChatEnter
)

// Base packet structure
type Packet struct {
	Type PacketType
	Size uint32
}

// ShipUpdate packet - matches C++ struct layout
type ShipUpdate struct {
	Packet
	ShipID  int32
	PosX    int32
	PosY    int32
	PosZ    int32
	RotX    int32
	RotY    int32
	RotZ    int32
	RotW    int32
	UpX     int32
	UpY     int32
	UpZ     int32
	UpW     int32
	LookAtX int32
	LookAtY int32
	LookAtZ int32
}

// Connect packet
type Connect struct {
	Packet
	Model int16
	Name  [64]byte
}

// ConnectAcknowledge packet
type ConnectAcknowledge struct {
	Packet
	ClientID int32
	ShipID   int32
}

// CreateShip packet
type CreateShip struct {
	Packet
	ShipID  int32
	Model   int32
	Owner   int32
	PosX    int32
	PosY    int32
	PosZ    int32
	RotX    int32
	RotY    int32
	RotZ    int32
	RotW    int32
	UpX     int32
	UpY     int32
	UpZ     int32
	UpW     int32
	LookAtX int32
	LookAtY int32
	LookAtZ int32
}

// DeleteShip packet
type DeleteShip struct {
	Packet
	ShipID int32
}

// ChatMessage packet
type ChatMessage struct {
	Packet
	A       uint8
	R       uint8
	G       uint8
	B       uint8
	Message [512]byte
}

// PlayerChatEnter packet
type PlayerChatEnter struct {
	Packet
	Message [512]byte
}

// Player represents a connected player
type Player struct {
	ID       int32
	Conn     *net.UDPConn
	Addr     *net.UDPAddr
	Name     string
	ShipID   int32
	LastSeen time.Time
}

// Server represents the game server
type Server struct {
	config     Config
	conn       *net.UDPConn
	players    map[int32]*Player
	nextID     int32
	nextShipID int32
	mu         sync.RWMutex
	shutdown   chan struct{}
}

// NewServer creates a new server instance
func NewServer(config Config) *Server {
	return &Server{
		config:     config,
		players:    make(map[int32]*Player),
		nextID:     1,
		nextShipID: 1000,
		shutdown:   make(chan struct{}),
	}
}

// Start initializes and starts the server
func (s *Server) Start() error {
	addr, err := net.ResolveUDPAddr("udp", fmt.Sprintf(":%d", s.config.ServerPort))
	if err != nil {
		return fmt.Errorf("failed to resolve UDP address: %w", err)
	}

	s.conn, err = net.ListenUDP("udp", addr)
	if err != nil {
		return fmt.Errorf("failed to start UDP listener: %w", err)
	}

	log.Printf("X3MP Go Server started on port %d", s.config.ServerPort)
	log.Printf("Server: %s | Max Players: %d | Tick Rate: %d", 
		s.config.ServerName, s.config.MaxPlayers, s.config.TickRate)

	// Start the game loop in a separate goroutine
	go s.gameLoop()
	
	// Start handling incoming packets
	return s.handlePackets()
}

// gameLoop runs the main server game loop
func (s *Server) gameLoop() {
	ticker := time.NewTicker(time.Second / time.Duration(s.config.TickRate))
	defer ticker.Stop()

	for {
		select {
		case <-s.shutdown:
			return
		case <-ticker.C:
			s.updatePlayers()
		}
	}
}

// updatePlayers handles periodic player updates and cleanup
func (s *Server) updatePlayers() {
	s.mu.Lock()
	defer s.mu.Unlock()

	now := time.Now()
	// Clean up disconnected players (no activity for 30 seconds)
	for id, player := range s.players {
		if now.Sub(player.LastSeen) > 30*time.Second {
			log.Printf("Player %d (%s) timed out, removing", id, player.Name)
			delete(s.players, id)
		}
	}
}

// handlePackets processes incoming network packets
func (s *Server) handlePackets() error {
	buffer := make([]byte, 4096)

	for {
		select {
		case <-s.shutdown:
			return nil
		default:
			n, addr, err := s.conn.ReadFromUDP(buffer)
			if err != nil {
				log.Printf("Error reading UDP packet: %v", err)
				continue
			}

			go s.processPacket(buffer[:n], addr)
		}
	}
}

// processPacket processes a single incoming packet
func (s *Server) processPacket(data []byte, addr *net.UDPAddr) {
	if len(data) < 5 { // Minimum packet size (type + size)
		log.Printf("Received packet too small from %s", addr)
		return
	}

	packetType := PacketType(data[0])
	size := binary.LittleEndian.Uint32(data[1:5])

	if int(size) != len(data) {
		log.Printf("Packet size mismatch from %s: expected %d, got %d", addr, size, len(data))
		return
	}

	switch packetType {
	case PacketConnect:
		s.handleConnect(data, addr)
	case PacketShipUpdate:
		s.handleShipUpdate(data, addr)
	case PacketChatMessage:
		s.handleChatMessage(data, addr)
	case PacketPlayerChatEnter:
		s.handlePlayerChatEnter(data, addr)
	default:
		log.Printf("Unknown packet type %d from %s", packetType, addr)
	}
}

// handleConnect processes player connection requests
func (s *Server) handleConnect(data []byte, addr *net.UDPAddr) {
	if len(data) < 71 { // Minimum size for Connect packet
		log.Printf("Connect packet too small from %s", addr)
		return
	}

	model := int16(binary.LittleEndian.Uint16(data[5:7]))
	name := string(data[7:71])
	
	// Clean the name (remove null bytes)
	if nullPos := findNull(name); nullPos != -1 {
		name = name[:nullPos]
	}

	s.mu.Lock()
	
	// Check if server is full
	if len(s.players) >= s.config.MaxPlayers {
		s.mu.Unlock()
		log.Printf("Server full, rejecting connection from %s", addr)
		return
	}

	clientID := s.nextID
	shipID := s.nextShipID
	s.nextID++
	s.nextShipID++

	player := &Player{
		ID:       clientID,
		Addr:     addr,
		Name:     name,
		ShipID:   shipID,
		LastSeen: time.Now(),
	}

	s.players[clientID] = player
	s.mu.Unlock()

	log.Printf("Player %d (%s) connected from %s with model %d", clientID, name, addr, model)

	// Send connection acknowledgment
	s.sendConnectAcknowledge(clientID, shipID, addr)
	
	// Broadcast new ship creation to all players
	s.broadcastCreateShip(shipID, int32(model), clientID, 0, 0, 0)
	
	// Send existing ships to new player
	s.sendExistingShips(addr)
}

// handleShipUpdate processes ship position updates
func (s *Server) handleShipUpdate(data []byte, addr *net.UDPAddr) {
	if len(data) < 65 { // Size of ShipUpdate packet
		return
	}

	// Find player by address
	s.mu.RLock()
	var player *Player
	for _, p := range s.players {
		if p.Addr.String() == addr.String() {
			player = p
			player.LastSeen = time.Now()
			break
		}
	}
	s.mu.RUnlock()

	if player == nil {
		log.Printf("Ship update from unknown player %s", addr)
		return
	}

	// Broadcast ship update to all other players
	s.broadcastToOthers(data, player.ID)
}

// handleChatMessage processes chat messages
func (s *Server) handleChatMessage(data []byte, addr *net.UDPAddr) {
	if len(data) < 521 { // Size of ChatMessage packet
		return
	}

	// Find player
	s.mu.RLock()
	var player *Player
	for _, p := range s.players {
		if p.Addr.String() == addr.String() {
			player = p
			player.LastSeen = time.Now()
			break
		}
	}
	s.mu.RUnlock()

	if player == nil {
		return
	}

	message := string(data[9:521])
	if nullPos := findNull(message); nullPos != -1 {
		message = message[:nullPos]
	}

	log.Printf("Chat from %s: %s", player.Name, message)
	
	// Broadcast to all players
	s.broadcast(data)
}

// handlePlayerChatEnter processes player chat enter events
func (s *Server) handlePlayerChatEnter(data []byte, addr *net.UDPAddr) {
	// Similar to handleChatMessage but for chat enter events
	s.broadcast(data)
}

// sendConnectAcknowledge sends connection acknowledgment to a player
func (s *Server) sendConnectAcknowledge(clientID, shipID int32, addr *net.UDPAddr) {
	packet := make([]byte, 13) // ConnectAcknowledge packet size
	packet[0] = byte(PacketConnectAcknowledge)
	binary.LittleEndian.PutUint32(packet[1:5], 13)
	binary.LittleEndian.PutUint32(packet[5:9], uint32(clientID))
	binary.LittleEndian.PutUint32(packet[9:13], uint32(shipID))

	_, err := s.conn.WriteToUDP(packet, addr)
	if err != nil {
		log.Printf("Error sending connect acknowledge to %s: %v", addr, err)
	}
}

// broadcastCreateShip broadcasts ship creation to all players
func (s *Server) broadcastCreateShip(shipID, model, owner, posX, posY, posZ int32) {
	packet := make([]byte, 73) // CreateShip packet size
	packet[0] = byte(PacketCreateShip)
	binary.LittleEndian.PutUint32(packet[1:5], 73)
	binary.LittleEndian.PutUint32(packet[5:9], uint32(shipID))
	binary.LittleEndian.PutUint32(packet[9:13], uint32(model))
	binary.LittleEndian.PutUint32(packet[13:17], uint32(owner))
	binary.LittleEndian.PutUint32(packet[17:21], uint32(posX))
	binary.LittleEndian.PutUint32(packet[21:25], uint32(posY))
	binary.LittleEndian.PutUint32(packet[25:29], uint32(posZ))
	// Set rotation and orientation to identity/default values
	binary.LittleEndian.PutUint32(packet[29:33], 0) // RotX
	binary.LittleEndian.PutUint32(packet[33:37], 0) // RotY
	binary.LittleEndian.PutUint32(packet[37:41], 0) // RotZ
	binary.LittleEndian.PutUint32(packet[41:45], 1) // RotW

	s.broadcast(packet)
}

// sendExistingShips sends information about existing ships to a new player
func (s *Server) sendExistingShips(addr *net.UDPAddr) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	for _, player := range s.players {
		if player.Addr.String() == addr.String() {
			continue // Don't send to the new player themselves
		}
		
		// Send CreateShip for each existing player
		packet := make([]byte, 73)
		packet[0] = byte(PacketCreateShip)
		binary.LittleEndian.PutUint32(packet[1:5], 73)
		binary.LittleEndian.PutUint32(packet[5:9], uint32(player.ShipID))
		binary.LittleEndian.PutUint32(packet[9:13], 1) // Default model
		binary.LittleEndian.PutUint32(packet[13:17], uint32(player.ID))
		
		_, err := s.conn.WriteToUDP(packet, addr)
		if err != nil {
			log.Printf("Error sending existing ship to %s: %v", addr, err)
		}
	}
}

// broadcast sends a packet to all connected players
func (s *Server) broadcast(data []byte) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	for _, player := range s.players {
		_, err := s.conn.WriteToUDP(data, player.Addr)
		if err != nil {
			log.Printf("Error broadcasting to player %d: %v", player.ID, err)
		}
	}
}

// broadcastToOthers sends a packet to all players except the specified one
func (s *Server) broadcastToOthers(data []byte, excludeID int32) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	for _, player := range s.players {
		if player.ID != excludeID {
			_, err := s.conn.WriteToUDP(data, player.Addr)
			if err != nil {
				log.Printf("Error broadcasting to player %d: %v", player.ID, err)
			}
		}
	}
}

// Stop gracefully shuts down the server
func (s *Server) Stop() {
	close(s.shutdown)
	if s.conn != nil {
		s.conn.Close()
	}
}

// findNull finds the first null byte in a string
func findNull(s string) int {
	for i, b := range []byte(s) {
		if b == 0 {
			return i
		}
	}
	return -1
}

// loadConfig loads configuration from JSON file
func loadConfig(filename string) (Config, error) {
	config := Config{
		ServerPort: 13337,
		MaxPlayers: 32,
		ServerName: "X3MP Go Server",
		LogLevel:   "info",
		TickRate:   60,
	}

	file, err := os.Open(filename)
	if err != nil {
		// If config file doesn't exist, create it with defaults
		if os.IsNotExist(err) {
			return config, saveConfig(filename, config)
		}
		return config, err
	}
	defer file.Close()

	decoder := json.NewDecoder(file)
	err = decoder.Decode(&config)
	return config, err
}

// saveConfig saves configuration to JSON file
func saveConfig(filename string, config Config) error {
	file, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	encoder := json.NewEncoder(file)
	encoder.SetIndent("", "  ")
	return encoder.Encode(config)
}

func main() {
	// Load configuration
	config, err := loadConfig("server_config.json")
	if err != nil {
		log.Printf("Warning: Could not load config: %v, using defaults", err)
	}

	// Create and start server
	server := NewServer(config)
	
	log.Println("==================================================================")
	log.Println("      X3MP Go Server starting...")
	log.Println("==================================================================")

	if err := server.Start(); err != nil {
		log.Fatalf("Failed to start server: %v", err)
	}
}