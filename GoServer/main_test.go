package main

import (
	"net"
	"reflect"
	"testing"

	"x3mp_goserver/network"
)

// TestPacketSerialization creates a packet, serializes it, deserializes it,
// and checks if the result matches the original.
func TestPacketSerialization(t *testing.T) {
	// Create a sample packet
	originalPkt := network.ConnectPacket{
		Header: network.PacketHeader{
			Type: network.Connect,
			Size: uint32(74), // Manually calculated size for this test
		},
		Model: 123,
	}
	copy(originalPkt.Name[:], "TestPlayer")

	// Serialize the packet
	serializedData, err := network.ToBytes(&originalPkt)
	if err != nil {
		t.Fatalf("Failed to serialize packet: %v", err)
	}

	// Deserialize the packet
	var deserializedPkt network.ConnectPacket
	err = network.FromBytes(serializedData, &deserializedPkt)
	if err != nil {
		t.Fatalf("Failed to deserialize packet: %v", err)
	}

	// Compare the original and deserialized packets
	if !reflect.DeepEqual(originalPkt, deserializedPkt) {
		t.Errorf("Deserialized packet does not match original packet.")
		t.Errorf("Original: %+v", originalPkt)
		t.Errorf("Deserialized: %+v", deserializedPkt)
	}
}

// TestHandleConnect simulates a client connection and checks if the server state is updated correctly.
func TestHandleConnect(t *testing.T) {
	// 1. Setup the server
	server := NewServer()
	initialEntityCount := len(server.universe.Entities)

	// 2. Create a fake client connection
	// Create a fake connect packet
	connectPkt := network.ConnectPacket{
		Header: network.PacketHeader{Type: network.Connect, Size: 74},
		Model:  99,
	}
	copy(connectPkt.Name[:], "TestHandleConnect")

	// Serialize the packet to create the raw data for the handler
	packetBytes, err := network.ToBytes(&connectPkt)
	if err != nil {
		t.Fatalf("Failed to serialize connect packet for test: %v", err)
	}

	// Create a fake remote address for the client
	fakeAddr := &net.UDPAddr{IP: net.ParseIP("127.0.0.1"), Port: 12345}

	// 3. Call the handler function
	// We need to mock the sendPacket and broadcastPacket methods for this to be a pure unit test,
	// but for now we will call it directly and ignore the network writes.
	server.handleConnect(fakeAddr, packetBytes)

	// 4. Assert the results
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
