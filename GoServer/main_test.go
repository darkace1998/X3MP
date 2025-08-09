package main

import (
	"encoding/binary"
	"net"
	"reflect"
	"testing"
	"time"

	"x3mp_goserver/game"
	"x3mp_goserver/network"
)

func TestPacketSerialization(t *testing.T) {
	connectPkt := network.ConnectPacket{
		Header: network.PacketHeader{
			Type: network.Connect,
			Size: uint32(binary.Size(network.ConnectPacket{})),
		},
		Model: 123,
	}
	copy(connectPkt.Name[:], "TestPlayer")

	testCases := []struct {
		name         string
		inputPacket  interface{}
		outputPacket interface{}
	}{
		{
			name:         "ConnectPacket",
			inputPacket:  &connectPkt,
			outputPacket: &network.ConnectPacket{},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			// Serialize the packet
			serializedData, err := network.ToBytes(tc.inputPacket)
			if err != nil {
				t.Fatalf("Failed to serialize packet: %v", err)
			}

			// Deserialize the packet
			err = network.FromBytes(serializedData, tc.outputPacket)
			if err != nil {
				t.Fatalf("Failed to deserialize packet: %v", err)
			}

			// Compare the original and deserialized packets
			if !reflect.DeepEqual(tc.inputPacket, tc.outputPacket) {
				t.Errorf("Deserialized packet does not match original packet.")
				t.Errorf("Original:   %+v", tc.inputPacket)
				t.Errorf("Deserialized: %+v", tc.outputPacket)
			}
		})
	}
}

func TestHandleConnect_ExistingShipBroadcast(t *testing.T) {
	// 1. Setup the server and manually add an existing ship
	server := NewServer()
	existingShipID := int32(10)
	server.universe.Entities[existingShipID] = &game.Entity{
		Model:      5,
		NetOwnerID: NoOwnerID, // No owner
		PosX:       1000,
		PosY:       2000,
		PosZ:       3000,
	}

	// 2. Create a fake client connection
	connectPkt := network.ConnectPacket{
		Header: network.PacketHeader{Type: network.Connect, Size: uint32(binary.Size(network.ConnectPacket{}))},
		Model:  99,
	}
	packetBytes, err := network.ToBytes(&connectPkt)
	if err != nil {
		t.Fatalf("Failed to serialize connect packet: %v", err)
	}

	// 3. Setup a listener for the fake client
	clientConn, err := net.ListenUDP("udp", &net.UDPAddr{IP: net.ParseIP("127.0.0.1"), Port: 0})
	if err != nil {
		t.Fatalf("Failed to listen on UDP for test: %v", err)
	}
	defer clientConn.Close()
	clientAddr := clientConn.LocalAddr().(*net.UDPAddr)

	// The server needs a connection to send from, but it doesn't have to be the same one
	server.conn, err = net.ListenUDP("udp", &net.UDPAddr{IP: net.ParseIP("127.0.0.1"), Port: 0})
	if err != nil {
		t.Fatalf("Failed to listen on UDP for test: %v", err)
	}
	defer server.conn.Close()

	// 4. Call the handler and read the packets
	readFinished := make(chan *network.CreateShipPacket)
	go func() {
		// We expect a ConnectAcknowledge packet first, then a CreateShip packet
		// for the existing ship.
		buf := make([]byte, 1024)
		_, _, err := clientConn.ReadFromUDP(buf) // Read and discard ConnectAcknowledge
		if err != nil {
			t.Logf("Read error (ConnectAcknowledge): %v", err)
			close(readFinished)
			return
		}

		n, _, err := clientConn.ReadFromUDP(buf)
		if err != nil {
			t.Logf("Read error: %v", err)
			close(readFinished)
			return
		}

		var createShipPkt network.CreateShipPacket
		err = network.FromBytes(buf[:n], &createShipPkt)
		if err != nil {
			t.Logf("Decode error: %v", err)
			close(readFinished)
			return
		}
		readFinished <- &createShipPkt
	}()

	server.handleConnect(clientAddr, packetBytes)

	select {
	case receivedPkt := <-readFinished:
		if receivedPkt == nil {
			t.Fatal("Failed to receive or decode CreateShipPacket")
		}
		if receivedPkt.ShipID != existingShipID {
			t.Errorf("Expected ShipID %d, got %d", existingShipID, receivedPkt.ShipID)
		}
		if receivedPkt.PosX != 1000 {
			t.Errorf("Expected PosX to be 1000, but got %d", receivedPkt.PosX)
		}
	case <-time.After(2 * time.Second):
		t.Fatal("Timed out waiting for CreateShipPacket")
	}
}

// TestHandleConnect simulates a client connection and checks if the server state is updated correctly.
func TestHandleConnect(t *testing.T) {
	t.Log("Starting TestHandleConnect")
	// 1. Setup the server
	server := NewServer()
	initialEntityCount := len(server.universe.Entities)

	// 2. Create a fake client connection
	// Create a fake connect packet
	connectPkt := network.ConnectPacket{
		Header: network.PacketHeader{Type: network.Connect, Size: uint32(binary.Size(network.ConnectPacket{}))},
		Model:  99,
	}
	copy(connectPkt.Name[:], "TestHandleConnect")

	// Serialize the packet to create the raw data for the handler
	packetBytes, err := network.ToBytes(&connectPkt)
	if err != nil {
		t.Fatalf("Failed to serialize connect packet for test: %v", err)
	}

	// Create a dummy connection for the server, which will also serve as our fake client connection
	addr := &net.UDPAddr{IP: net.ParseIP("127.0.0.1"), Port: 0} // Port 0 asks the OS for a free port
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		t.Fatalf("Failed to listen on UDP for test: %v", err)
	}
	fakeAddr := conn.LocalAddr().(*net.UDPAddr)
	defer conn.Close()
	server.conn = conn

	// 4. Call the handler function
	// We need to read the packets the server sends back to avoid blocking
	readFinished := make(chan bool)
	go func() {
		buf := make([]byte, 1024)
		// Expect ConnectAcknowledge
		_, _, err := conn.ReadFromUDP(buf)
		if err != nil {
			t.Errorf("Failed to read ConnectAcknowledge packet: %v", err)
			readFinished <- true
			return
		}
		// Expect CreateShip for initial ship
		_, _, err = conn.ReadFromUDP(buf)
		if err != nil {
			t.Errorf("Failed to read CreateShip packet: %v", err)
			readFinished <- true
			return
		}
		readFinished <- true
	}()

	server.handleConnect(fakeAddr, packetBytes)

	// Wait for the reads to complete before asserting, with a timeout
	select {
	case <-readFinished:
		// all good
	case <-time.After(1 * time.Second):
		t.Fatal("timed out waiting for server to send packets")
	}

	// 5. Assert the results
	// Check if a new client was added
	if len(server.clients) != 1 {
		t.Errorf("Expected 1 client after connect, but got %d", len(server.clients))
	}

	// Check if a new entity (ship) was created
	if len(server.universe.Entities) != initialEntityCount+1 {
		t.Errorf("Expected %d entities after connect, but got %d", initialEntityCount+1, len(server.universe.Entities))
	}

	// Check if the client was stored correctly
	client, ok := server.clients[fakeAddr.String()]
	if !ok {
		t.Fatalf("Client with address %s was not found in the server's client map", fakeAddr.String())
	}

	if client.ClientID != 0 {
		t.Errorf("Expected new client to have ClientID 0, but got %d", client.ClientID)
	}

	// Check if the new ship is owned by the new client
	newShip, ok := server.universe.Entities[client.ShipID]
	if !ok {
		t.Fatalf("Client's ship with ShipID %d was not found in the universe", client.ShipID)
	}

	if newShip.NetOwnerID != client.ClientID {
		t.Errorf("New ship's owner ID does not match client's ID. Expected %d, got %d", client.ClientID, newShip.NetOwnerID)
	}
}

func TestHandleShipUpdate(t *testing.T) {
	type testCase struct {
		name         string
		initialPosX  int32
		updatePacket network.ShipUpdatePacket
		expectedPosX int32
	}

	server := NewServer()
	clientAddr := &net.UDPAddr{IP: net.ParseIP("127.0.0.1"), Port: 54321}
	clientID := int32(1)
	shipID := int32(5)

	server.clients[clientAddr.String()] = &Client{
		Addr:     clientAddr,
		ClientID: clientID,
		ShipID:   shipID,
	}
	server.universe.Entities[shipID] = &game.Entity{
		Model:      1,
		NetOwnerID: clientID,
	}

	testCases := []testCase{
		{
			name:        "Basic position update",
			initialPosX: 0,
			updatePacket: network.ShipUpdatePacket{
				Header: network.PacketHeader{Type: network.ShipUpdate, Size: uint32(binary.Size(network.ShipUpdatePacket{}))},
				ShipID: shipID,
				PosX:   100,
				PosY:   200,
				PosZ:   300,
			},
			expectedPosX: 100,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			// Set initial state
			server.universe.Entities[shipID].PosX = tc.initialPosX

			// Serialize the packet
			packetBytes, err := network.ToBytes(&tc.updatePacket)
			if err != nil {
				t.Fatalf("Failed to serialize ship update packet: %v", err)
			}

			// Call the handler
			server.handleShipUpdate(clientAddr, packetBytes)

			// Assert the results
			updatedShip, ok := server.universe.Entities[shipID]
			if !ok {
				t.Fatalf("Ship with ID %d not found after update", shipID)
			}

			if updatedShip.PosX != tc.expectedPosX {
				t.Errorf("Expected PosX to be %d, but got %d", tc.expectedPosX, updatedShip.PosX)
			}
		})
	}
}
