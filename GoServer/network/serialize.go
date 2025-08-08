package network

import (
	"bytes"
	"encoding/binary"
)

// ToBytes serializes a packet struct into a byte slice.
// It uses Little Endian byte order.
func ToBytes(p interface{}) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, p)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

// FromBytes deserializes a byte slice into a packet struct.
// It uses Little Endian byte order.
func FromBytes(data []byte, p interface{}) error {
	buf := bytes.NewReader(data)
	err := binary.Read(buf, binary.LittleEndian, p)
	if err != nil {
		return err
	}
	return nil
}

// DecodeHeader deserializes just the header from a byte slice
// to allow for peeking at the packet type and size.
func DecodeHeader(data []byte) (*PacketHeader, error) {
	var header PacketHeader
	reader := bytes.NewReader(data)
	err := binary.Read(reader, binary.LittleEndian, &header)
	if err != nil {
		return nil, err
	}
	return &header, nil
}
