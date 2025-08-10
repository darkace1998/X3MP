package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"net"
	"testing"
	"time"
	"x3mp_goserver/network"
)

// This file will contain the compatibility tests.

// serializePacket manually serializes a packet struct into a byte slice that
// is compatible with the C++ client's expected memory layout.
// C++ Layout: [PacketType (int32)][Size (uint32)][Payload...]
func serializePacket(p interface{}) ([]byte, error) {
	buf := new(bytes.Buffer)

	switch pkt := p.(type) {
	case network.ConnectPacket:
		// Define type and size for this packet.
		packetType := network.Connect
		// Size = sizeof(type) + sizeof(size) + sizeof(model) + sizeof(name)
		packetSize := uint32(4 + 4 + 2 + 64)

		// Write fields in order.
		if err := binary.Write(buf, binary.LittleEndian, packetType); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, packetSize); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.Model); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.Name); err != nil {
			return nil, err
		}

		return buf.Bytes(), nil

	case network.ShipUpdatePacket:
		packetType := network.ShipUpdate
		// Size = type(4) + size(4) + 15*int32(60) = 68
		packetSize := uint32(4 + 4 + (15 * 4))

		if err := binary.Write(buf, binary.LittleEndian, packetType); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, packetSize); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.ShipID); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.PosX); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.PosY); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.PosZ); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.RotX); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.RotY); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.RotZ); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.RotW); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.UpX); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.UpY); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.UpZ); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.UpW); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.LookAtX); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.LookAtY); err != nil {
			return nil, err
		}
		if err := binary.Write(buf, binary.LittleEndian, pkt.LookAtZ); err != nil {
			return nil, err
		}

		return buf.Bytes(), nil

	default:
		return nil, fmt.Errorf("unsupported packet type for manual serialization: %T", p)
	}
}

func TestServerGameStateUpdate(t *testing.T) {
	// 1. Setup server and connect a client
	server := NewServer()
	go func() {
		if err := server.Listen(); err != nil {
			t.Errorf("Server listen failed: %v", err)
		}
	}()
	time.Sleep(100 * time.Millisecond)
	defer server.Stop()

	conn, err := net.Dial("udp", "127.0.0.1:13337")
	if err != nil {
		t.Fatalf("Failed to connect to server: %v", err)
	}
	defer conn.Close()

	connectPkt := network.ConnectPacket{Model: 1}
	copy(connectPkt.Name[:], "TestClientForUpdate")
	packetBytes, _ := serializePacket(connectPkt)
	if _, err := conn.Write(packetBytes); err != nil {
		t.Fatalf("Failed to write connect packet: %v", err)
	}

	response := make([]byte, 1024)
	n, err := conn.Read(response)
	if err != nil {
		t.Fatalf("Failed to read connect response: %v", err)
	}

	var ackPkt network.ConnectAcknowledgePacket
	if err := network.FromBytes(response[:n], &ackPkt); err != nil {
		t.Fatalf("Failed to deserialize response: %v", err)
	}

	shipID := ackPkt.ShipID

	// 2. Create and send a ShipUpdate packet
	updatePkt := network.ShipUpdatePacket{
		ShipID: shipID,
		PosX:   100,
		PosY:   200,
		PosZ:   300,
	}

	updateBytes, err := serializePacket(updatePkt)
	if err != nil {
		t.Fatalf("Failed to serialize ship update packet: %v", err)
	}

	_, err = conn.Write(updateBytes)
	if err != nil {
		t.Fatalf("Failed to send ship update packet: %v", err)
	}

	// Give server time to process the update
	time.Sleep(50 * time.Millisecond)

	// 3. Verify server state
	ship, ok := server.universe.GetEntity(shipID)
	if !ok {
		t.Fatalf("Ship with ID %d not found in server universe", shipID)
	}

	if ship.PosX != 100 || ship.PosY != 200 || ship.PosZ != 300 {
		t.Errorf("Ship position was not updated correctly. Got PosX=%d, PosY=%d, PosZ=%d", ship.PosX, ship.PosY, ship.PosZ)
	}
}

func TestServerClientConnection(t *testing.T) {
	// 1. Setup and run the server in a goroutine
	server := NewServer()
	go func() {
		if err := server.Listen(); err != nil {
			t.Errorf("Server listen failed: %v", err)
		}
	}()
	time.Sleep(100 * time.Millisecond) // Give server time to start
	defer server.Stop()

	// 2. Create a mock client
	conn, err := net.Dial("udp", "127.0.0.1:13337")
	if err != nil {
		t.Fatalf("Failed to connect to server: %v", err)
	}
	defer conn.Close()

	// 3. Create and serialize a Connect packet
	connectPkt := network.ConnectPacket{
		Model: 1,
	}
	copy(connectPkt.Name[:], "CompatTestClient")

	packetBytes, err := serializePacket(connectPkt)
	if err != nil {
		t.Fatalf("Failed to serialize connect packet: %v", err)
	}

	// 4. Send the packet
	_, err = conn.Write(packetBytes)
	if err != nil {
		t.Fatalf("Failed to send connect packet: %v", err)
	}

	// 5. Wait for a response
	response := make([]byte, 1024)
	n, err := conn.Read(response)
	if err != nil {
		t.Fatalf("Failed to read response from server: %v", err)
	}

	// 6. Deserialize and verify the response
	var ackPkt network.ConnectAcknowledgePacket
	// The server sends back a standard Go-serialized struct, so FromBytes should work.
	err = network.FromBytes(response[:n], &ackPkt)
	if err != nil {
		t.Fatalf("Failed to deserialize connect acknowledge packet: %v", err)
	}

	if ackPkt.Header.Type != network.ConnectAcknowledge {
		t.Errorf("Expected ConnectAcknowledge packet, but got type %d", ackPkt.Header.Type)
	}

	if ackPkt.ClientID != 0 {
		t.Errorf("Expected ClientID 0, but got %d", ackPkt.ClientID)
	}

	// 7. Also check server state (with mutex to prevent race conditions)
	server.clientsMutex.RLock()
	clientCount := len(server.clients)
	server.clientsMutex.RUnlock()
	if clientCount != 1 {
		t.Errorf("Expected 1 client on server, but got %d", clientCount)
	}
}
