#include <gtest/gtest.h>
#include "net_packets.h"
#include <cstring>

// Mock network serialization functions for testing
namespace TestSerialization {
    template<typename T>
    std::vector<uint8_t> SerializePacket(const T& packet) {
        std::vector<uint8_t> data(sizeof(T));
        std::memcpy(data.data(), &packet, sizeof(T));
        return data;
    }
    
    template<typename T>
    T DeserializePacket(const std::vector<uint8_t>& data) {
        T packet;
        if (data.size() >= sizeof(T)) {
            std::memcpy(&packet, data.data(), sizeof(T));
        }
        return packet;
    }
}

class PacketSerializationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test data
    }
};

TEST_F(PacketSerializationTest, ShipUpdateSerialization) {
    x3::net::ShipUpdate original;
    original.type = x3::net::PacketType::ShipUpdate;
    original.size = sizeof(x3::net::ShipUpdate);
    original.ShipID = 12345;
    original.PosX = 1000;
    original.PosY = 2000;
    original.PosZ = 3000;
    original.RotX = 100;
    original.RotY = 200;
    original.RotZ = 300;
    original.RotW = 400;
    original.UpX = 10;
    original.UpY = 20;
    original.UpZ = 30;
    original.UpW = 40;
    original.LookAtX = 500;
    original.LookAtY = 600;
    original.LookAtZ = 700;
    
    // Serialize
    auto serialized = TestSerialization::SerializePacket(original);
    
    // Deserialize
    auto deserialized = TestSerialization::DeserializePacket<x3::net::ShipUpdate>(serialized);
    
    // Verify all fields
    EXPECT_EQ(deserialized.type, original.type);
    EXPECT_EQ(deserialized.size, original.size);
    EXPECT_EQ(deserialized.ShipID, original.ShipID);
    EXPECT_EQ(deserialized.PosX, original.PosX);
    EXPECT_EQ(deserialized.PosY, original.PosY);
    EXPECT_EQ(deserialized.PosZ, original.PosZ);
    EXPECT_EQ(deserialized.RotX, original.RotX);
    EXPECT_EQ(deserialized.RotY, original.RotY);
    EXPECT_EQ(deserialized.RotZ, original.RotZ);
    EXPECT_EQ(deserialized.RotW, original.RotW);
    EXPECT_EQ(deserialized.UpX, original.UpX);
    EXPECT_EQ(deserialized.UpY, original.UpY);
    EXPECT_EQ(deserialized.UpZ, original.UpZ);
    EXPECT_EQ(deserialized.UpW, original.UpW);
    EXPECT_EQ(deserialized.LookAtX, original.LookAtX);
    EXPECT_EQ(deserialized.LookAtY, original.LookAtY);
    EXPECT_EQ(deserialized.LookAtZ, original.LookAtZ);
}

TEST_F(PacketSerializationTest, ConnectSerialization) {
    x3::net::Connect original;
    original.type = x3::net::PacketType::Connect;
    original.size = sizeof(x3::net::Connect);
    original.Model = 42;
    std::strncpy(original.Name, "TestPlayer", sizeof(original.Name) - 1);
    original.Name[sizeof(original.Name) - 1] = '\0';
    
    // Serialize
    auto serialized = TestSerialization::SerializePacket(original);
    
    // Deserialize
    auto deserialized = TestSerialization::DeserializePacket<x3::net::Connect>(serialized);
    
    // Verify all fields
    EXPECT_EQ(deserialized.type, original.type);
    EXPECT_EQ(deserialized.size, original.size);
    EXPECT_EQ(deserialized.Model, original.Model);
    EXPECT_STREQ(deserialized.Name, original.Name);
}

TEST_F(PacketSerializationTest, ConnectAcknowledgeSerialization) {
    x3::net::ConnectAcknowledge original;
    original.type = x3::net::PacketType::ConnectAcknowledge;
    original.size = sizeof(x3::net::ConnectAcknowledge);
    original.ClientID = 999;
    original.ShipID = 777;
    
    // Serialize
    auto serialized = TestSerialization::SerializePacket(original);
    
    // Deserialize
    auto deserialized = TestSerialization::DeserializePacket<x3::net::ConnectAcknowledge>(serialized);
    
    // Verify all fields
    EXPECT_EQ(deserialized.type, original.type);
    EXPECT_EQ(deserialized.size, original.size);
    EXPECT_EQ(deserialized.ClientID, original.ClientID);
    EXPECT_EQ(deserialized.ShipID, original.ShipID);
}

TEST_F(PacketSerializationTest, CreateShipSerialization) {
    x3::net::CreateShip original;
    original.type = x3::net::PacketType::CreateShip;
    original.size = sizeof(x3::net::CreateShip);
    original.ShipID = 123;
    original.Model = 456;
    original.Owner = 789;
    original.PosX = 1000;
    original.PosY = 2000;
    original.PosZ = 3000;
    original.RotX = 100;
    original.RotY = 200;
    original.RotZ = 300;
    original.RotW = 400;
    original.UpX = 10;
    original.UpY = 20;
    original.UpZ = 30;
    original.UpW = 40;
    original.LookAtX = 500;
    original.LookAtY = 600;
    original.LookAtZ = 700;
    
    // Serialize
    auto serialized = TestSerialization::SerializePacket(original);
    
    // Deserialize
    auto deserialized = TestSerialization::DeserializePacket<x3::net::CreateShip>(serialized);
    
    // Verify key fields
    EXPECT_EQ(deserialized.type, original.type);
    EXPECT_EQ(deserialized.size, original.size);
    EXPECT_EQ(deserialized.ShipID, original.ShipID);
    EXPECT_EQ(deserialized.Model, original.Model);
    EXPECT_EQ(deserialized.Owner, original.Owner);
    EXPECT_EQ(deserialized.PosX, original.PosX);
    EXPECT_EQ(deserialized.PosY, original.PosY);
    EXPECT_EQ(deserialized.PosZ, original.PosZ);
}

TEST_F(PacketSerializationTest, DeleteShipSerialization) {
    x3::net::DeleteShip original;
    original.type = x3::net::PacketType::DeleteShip;
    original.size = sizeof(x3::net::DeleteShip);
    original.ShipID = 999;
    
    // Serialize
    auto serialized = TestSerialization::SerializePacket(original);
    
    // Deserialize
    auto deserialized = TestSerialization::DeserializePacket<x3::net::DeleteShip>(serialized);
    
    // Verify all fields
    EXPECT_EQ(deserialized.type, original.type);
    EXPECT_EQ(deserialized.size, original.size);
    EXPECT_EQ(deserialized.ShipID, original.ShipID);
}

TEST_F(PacketSerializationTest, PacketSizeConsistency) {
    // Verify that packet sizes are consistent and reasonable
    EXPECT_GT(sizeof(x3::net::ShipUpdate), sizeof(x3::net::Packet));
    EXPECT_GT(sizeof(x3::net::Connect), sizeof(x3::net::Packet));
    EXPECT_GT(sizeof(x3::net::ConnectAcknowledge), sizeof(x3::net::Packet));
    EXPECT_GT(sizeof(x3::net::CreateShip), sizeof(x3::net::Packet));
    EXPECT_GT(sizeof(x3::net::DeleteShip), sizeof(x3::net::Packet));
    
    // Ensure packets are reasonably sized (not too large)
    EXPECT_LT(sizeof(x3::net::ShipUpdate), 1024);
    EXPECT_LT(sizeof(x3::net::Connect), 1024);
    EXPECT_LT(sizeof(x3::net::CreateShip), 1024);
}