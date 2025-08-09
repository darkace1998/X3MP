package network

// PacketType defines the type of a network packet.
type PacketType int32

const (
	Connect PacketType = iota
	CreateShip
	DeleteShip
	CreateStar
	ShipUpdate
	ConnectAcknowledge
	ChatMessage
	PlayerChatEnter
)

// Packet is the base struct for all network packets.
// NOTE: The C++ code uses an empty Packet struct and then each packet
// has its own type and size. Here we are embedding the base packet,
// but for serialization we need to be careful to match the C++ layout.
// Let's adjust this to better match the C++ version for easier serialization.
// We will have a separate header for each packet.

// PacketHeader defines the header for each packet.
type PacketHeader struct {
	Type PacketType
	Size uint32
}

// ShipUpdatePacket corresponds to the C++ ShipUpdate struct.
type ShipUpdatePacket struct {
	Header  PacketHeader
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

// ConnectPacket corresponds to the C++ Connect struct.
type ConnectPacket struct {
	Header PacketHeader
	Model  int16
	Name   [64]byte
}

// ConnectAcknowledgePacket corresponds to the C++ ConnectAcknowledge struct.
type ConnectAcknowledgePacket struct {
	Header   PacketHeader
	ClientID int32
	ShipID   int32
}

// CreateShipPacket corresponds to the C++ CreateShip struct.
type CreateShipPacket struct {
	Header  PacketHeader
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

// DeleteShipPacket corresponds to the C++ DeleteShip struct.
type DeleteShipPacket struct {
	Header PacketHeader
	ShipID int32
}

// CreateStarPacket corresponds to the C++ CreateStar struct.
type CreateStarPacket struct {
	Header PacketHeader
	StarID int32
	Model  int32
	PosX   int32
	PosY   int32
	PosZ   int32
}

// ChatMessagePacket corresponds to the C++ ChatMessage struct.
type ChatMessagePacket struct {
	Header  PacketHeader
	A       uint8
	R       uint8
	G       uint8
	B       uint8
	Message [512]byte
}

// PlayerChatEnterPacket corresponds to the C++ PlayerChatEnter struct.
type PlayerChatEnterPacket struct {
	Header  PacketHeader
	Message [512]byte
}
