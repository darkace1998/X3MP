package main

import (
	"fmt"
	"log"
	"net"
	"sync"

	"x3mp_goserver/game"
	"x3mp_goserver/network"
)

const (
	Port      = 13337
	NoOwnerID = -1
)

// Client represents a connected player.
type Client struct {
	Addr     *net.UDPAddr
	ClientID int32
	ShipID   int32
}

// Server holds the state of the game server.
type Server struct {
	conn         *net.UDPConn
	clients      map[string]*Client // Map of address string to client
	clientsMutex sync.RWMutex
	universe     *game.Universe
	nextClientID int32
}

// NewServer creates and initializes a new server.
func NewServer() *Server {
	// Replicate the logic from the Lua script: create one ship on startup.
	universe := game.NewUniverse()
	universe.CreateShip(22, -1) // model 22, no owner

	return &Server{
		clients:      make(map[string]*Client),
		universe:     universe,
		nextClientID: 0,
	}
}

// Listen starts the server's listening loop.
func (s *Server) Listen() error {
	addr := &net.UDPAddr{
		Port: Port,
		IP:   net.ParseIP("0.0.0.0"),
	}
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		return fmt.Errorf("failed to listen on port %d: %w", Port, err)
	}
	s.conn = conn
	defer s.conn.Close()

	log.Printf("Server listening on port %d", Port)

	buffer := make([]byte, 1024)

	for {
		n, remoteAddr, err := s.conn.ReadFromUDP(buffer)
		if err != nil {
			log.Printf("Error reading from UDP: %v", err)
			continue
		}

		data := buffer[:n]
		header, err := network.DecodeHeader(data)
		if err != nil {
			log.Printf("Error decoding packet header from %s: %v", remoteAddr, err)
			continue
		}

		switch header.Type {
		case network.Connect:
			s.handleConnect(remoteAddr, data)
		case network.ShipUpdate:
			s.handleShipUpdate(remoteAddr, data)
		case network.ChatMessage:
			s.handleChatMessage(remoteAddr, data)
		case network.PlayerChatEnter:
			s.handlePlayerChatEnter(remoteAddr, data)
		case network.DeleteShip:
			s.handleDeleteShip(remoteAddr, data)
		case network.CreateStar:
			s.handleCreateStar(remoteAddr, data)
		default:
			log.Printf("Received unhandled packet type: %d from %s", header.Type, remoteAddr)
		}
	}
}

// Stop gracefully shuts down the server by closing the connection.
func (s *Server) Stop() {
	if s.conn != nil {
		if err := s.conn.Close(); err != nil {
			log.Printf("Error closing connection: %v", err)
		}
	}
}

func (s *Server) handleConnect(addr *net.UDPAddr, data []byte) {
	log.Printf("Handling Connect from %s", addr.String())
	var connectPkt network.ConnectPacket
	if err := network.FromBytes(data, &connectPkt); err != nil {
		log.Printf("Error decoding ConnectPacket: %v", err)
		return
	}

	s.clientsMutex.Lock()
	defer s.clientsMutex.Unlock()

	// Create and store the new client
	newClientID := s.nextClientID
	s.nextClientID++

	shipID, _ := s.universe.CreateShip(int32(connectPkt.Model), newClientID)

	client := &Client{
		Addr:     addr,
		ClientID: newClientID,
		ShipID:   shipID,
	}
	s.clients[addr.String()] = client

	log.Printf("Player connected: ClientID %d, ShipID %d", newClientID, shipID)

	// Send ConnectAcknowledge back to the new client
	ackPkt := network.ConnectAcknowledgePacket{
		Header:   network.PacketHeader{Type: network.ConnectAcknowledge, Size: uint32(16)}, // type+size+clientID+shipID
		ClientID: newClientID,
		ShipID:   shipID,
	}
	s.sendPacket(addr, &ackPkt)

	// Send all existing ships to the new client
	for existingShipID, entity := range s.universe.GetAllEntities() {
		// The original C++ code doesn't send the newly created ship back to the owner
		// in this loop, so we replicate that behavior.
		if existingShipID == shipID {
			continue
		}
		createShipPkt := network.CreateShipPacket{
			Header:  network.PacketHeader{Type: network.CreateShip, Size: uint32(68)},
			ShipID:  existingShipID,
			Model:   entity.Model,
			Owner:   entity.NetOwnerID,
			PosX:    entity.PosX,
			PosY:    entity.PosY,
			PosZ:    entity.PosZ,
			RotX:    entity.RotX,
			RotY:    entity.RotY,
			RotZ:    entity.RotZ,
			RotW:    entity.RotW,
			UpX:     entity.UpX,
			UpY:     entity.UpY,
			UpZ:     entity.UpZ,
			UpW:     entity.UpW,
			LookAtX: entity.LookAtX,
			LookAtY: entity.LookAtY,
			LookAtZ: entity.LookAtZ,
		}
		s.sendPacket(addr, &createShipPkt)
	}

	// Tell all other clients about the new ship
	newShipCreatePkt := network.CreateShipPacket{
		Header: network.PacketHeader{Type: network.CreateShip, Size: uint32(68)},
		ShipID: shipID,
		Model:  int32(connectPkt.Model),
		Owner:  newClientID,
	}
	s.broadcastPacket(&newShipCreatePkt, addr.String())
}

func (s *Server) handleShipUpdate(addr *net.UDPAddr, data []byte) {
	var updatePkt network.ShipUpdatePacket
	if err := network.FromBytes(data, &updatePkt); err != nil {
		log.Printf("Error decoding ShipUpdatePacket: %v", err)
		return
	}

	// Update the entity in our universe
	if entity, ok := s.universe.GetEntity(updatePkt.ShipID); ok {
		// Basic validation: does the sender own this ship?
		s.clientsMutex.RLock()
		client, clientOk := s.clients[addr.String()]
		s.clientsMutex.RUnlock()
		if clientOk && client.ClientID == entity.NetOwnerID {
			entity.PosX = updatePkt.PosX
			entity.PosY = updatePkt.PosY
			entity.PosZ = updatePkt.PosZ
			entity.RotX = updatePkt.RotX
			entity.RotY = updatePkt.RotY
			entity.RotZ = updatePkt.RotZ
			entity.RotW = updatePkt.RotW
			entity.UpX = updatePkt.UpX
			entity.UpY = updatePkt.UpY
			entity.UpZ = updatePkt.UpZ
			entity.UpW = updatePkt.UpW
			entity.LookAtX = updatePkt.LookAtX
			entity.LookAtY = updatePkt.LookAtY
			entity.LookAtZ = updatePkt.LookAtZ
		}
	}

	// Broadcast the update to all other clients
	s.broadcastPacket(&updatePkt, addr.String())
}

func (s *Server) sendPacket(addr *net.UDPAddr, pkt interface{}) {
	bytes, err := network.ToBytes(pkt)
	if err != nil {
		log.Printf("Error serializing packet for sending: %v", err)
		return
	}
	_, err = s.conn.WriteToUDP(bytes, addr)
	if err != nil {
		log.Printf("Error sending packet to %s: %v", addr, err)
	}
}

func (s *Server) broadcastPacket(pkt interface{}, exceptAddr string) {
	bytes, err := network.ToBytes(pkt)
	if err != nil {
		log.Printf("Error serializing packet for broadcast: %v", err)
		return
	}

	s.clientsMutex.RLock()
	defer s.clientsMutex.RUnlock()
	for addrStr, client := range s.clients {
		if addrStr == exceptAddr {
			continue
		}
		_, err = s.conn.WriteToUDP(bytes, client.Addr)
		if err != nil {
			log.Printf("Error broadcasting packet to %s: %v", client.Addr, err)
		}
	}
}

func (s *Server) handleChatMessage(addr *net.UDPAddr, data []byte) {
	var chatPkt network.ChatMessagePacket
	if err := network.FromBytes(data, &chatPkt); err != nil {
		log.Printf("Error decoding ChatMessagePacket: %v", err)
		return
	}

	log.Printf("Chat message from %s: %s", addr.String(), string(chatPkt.Message[:]))

	// Broadcast the chat message to all other clients
	s.broadcastPacket(&chatPkt, addr.String())
}

func (s *Server) handlePlayerChatEnter(addr *net.UDPAddr, data []byte) {
	var chatPkt network.PlayerChatEnterPacket
	if err := network.FromBytes(data, &chatPkt); err != nil {
		log.Printf("Error decoding PlayerChatEnterPacket: %v", err)
		return
	}

	log.Printf("Player chat enter from %s: %s", addr.String(), string(chatPkt.Message[:]))

	// Broadcast to all other clients
	s.broadcastPacket(&chatPkt, addr.String())
}

func (s *Server) handleDeleteShip(addr *net.UDPAddr, data []byte) {
	var deletePkt network.DeleteShipPacket
	if err := network.FromBytes(data, &deletePkt); err != nil {
		log.Printf("Error decoding DeleteShipPacket: %v", err)
		return
	}

	// Remove ship from universe
	s.universe.DeleteShip(deletePkt.ShipID)
	log.Printf("Ship deleted: %d", deletePkt.ShipID)

	// Broadcast to all clients
	s.broadcastPacket(&deletePkt, "")
}

func (s *Server) handleCreateStar(addr *net.UDPAddr, data []byte) {
	var starPkt network.CreateStarPacket
	if err := network.FromBytes(data, &starPkt); err != nil {
		log.Printf("Error decoding CreateStarPacket: %v", err)
		return
	}

	log.Printf("Star creation request from %s: StarID %d, Model %d", addr.String(), starPkt.StarID, starPkt.Model)

	// Broadcast to all clients
	s.broadcastPacket(&starPkt, "")
}

func main() {
	log.Println("Starting X3MP Go Server...")
	server := NewServer()
	if err := server.Listen(); err != nil {
		log.Fatalf("Server failed: %v", err)
	}
}
