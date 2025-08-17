#include <gtest/gtest.h>
#include "ReliabilityManager.h"
#include <vector>
#include <thread>
#include <chrono>

class ReliabilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<x3::net::ReliabilityManager>();
        receivedPackets.clear();
        
        // Set up packet callback
        manager->SetPacketCallback([this](const uint8_t* data, size_t size) {
            std::vector<uint8_t> packet(data, data + size);
            receivedPackets.push_back(std::move(packet));
        });
    }

    void TearDown() override {
        manager.reset();
    }

    std::unique_ptr<x3::net::ReliabilityManager> manager;
    std::vector<std::vector<uint8_t>> receivedPackets;
};

TEST_F(ReliabilityTest, BasicPacketPreparation) {
    std::string testData = "Hello, World!";
    auto packet = manager->PreparePacket(
        reinterpret_cast<const uint8_t*>(testData.c_str()),
        testData.length(),
        true
    );
    
    // Should have reliable header + payload
    EXPECT_GT(packet.size(), testData.length());
    EXPECT_EQ(packet.size(), sizeof(x3::net::ReliableHeader) + testData.length());
}

TEST_F(ReliabilityTest, PacketProcessingAndDelivery) {
    std::string testData = "Test message";
    
    // Prepare a packet
    auto packet = manager->PreparePacket(
        reinterpret_cast<const uint8_t*>(testData.c_str()),
        testData.length(),
        true
    );
    
    // Process the same packet (simulating receive)
    manager->ProcessReceivedPacket(packet.data(), packet.size());
    
    // Should have received one packet
    ASSERT_EQ(receivedPackets.size(), 1);
    
    // Verify the payload
    std::string receivedData(
        reinterpret_cast<const char*>(receivedPackets[0].data()),
        receivedPackets[0].size()
    );
    EXPECT_EQ(receivedData, testData);
}

TEST_F(ReliabilityTest, DuplicatePacketFiltering) {
    std::string testData = "Duplicate test";
    
    // Prepare a packet
    auto packet = manager->PreparePacket(
        reinterpret_cast<const uint8_t*>(testData.c_str()),
        testData.length(),
        true
    );
    
    // Process the packet twice
    manager->ProcessReceivedPacket(packet.data(), packet.size());
    manager->ProcessReceivedPacket(packet.data(), packet.size());
    
    // Should have received only one packet (duplicate filtered)
    EXPECT_EQ(receivedPackets.size(), 1);
    
    // Check statistics
    auto stats = manager->GetStats();
    EXPECT_EQ(stats.duplicatesReceived, 1);
}

TEST_F(ReliabilityTest, OutOfOrderDelivery) {
    // Create multiple packets
    std::vector<std::string> messages = {"First", "Second", "Third"};
    std::vector<std::vector<uint8_t>> packets;
    
    for (const auto& msg : messages) {
        auto packet = manager->PreparePacket(
            reinterpret_cast<const uint8_t*>(msg.c_str()),
            msg.length(),
            true
        );
        packets.push_back(std::move(packet));
    }
    
    // Process packets out of order (2, 3, 1)
    manager->ProcessReceivedPacket(packets[1].data(), packets[1].size());
    manager->ProcessReceivedPacket(packets[2].data(), packets[2].size());
    manager->ProcessReceivedPacket(packets[0].data(), packets[0].size());
    
    // Should have received all packets in correct order
    ASSERT_EQ(receivedPackets.size(), 3);
    
    // Verify order (should be delivered in sequence order)
    for (size_t i = 0; i < messages.size(); ++i) {
        std::string receivedData(
            reinterpret_cast<const char*>(receivedPackets[i].data()),
            receivedPackets[i].size()
        );
        EXPECT_EQ(receivedData, messages[i]);
    }
}

TEST_F(ReliabilityTest, ChecksumValidation) {
    std::string testData = "Checksum test";
    
    auto packet = manager->PreparePacket(
        reinterpret_cast<const uint8_t*>(testData.c_str()),
        testData.length(),
        true
    );
    
    // Corrupt the packet data (not the header)
    if (packet.size() > sizeof(x3::net::ReliableHeader)) {
        packet[sizeof(x3::net::ReliableHeader)] ^= 0xFF; // Flip bits
    }
    
    // Process corrupted packet
    manager->ProcessReceivedPacket(packet.data(), packet.size());
    
    // Should not have received any packets due to checksum failure
    EXPECT_EQ(receivedPackets.size(), 0);
}

TEST_F(ReliabilityTest, RetransmissionTimeout) {
    // Set a very short retransmission timeout for testing
    manager->SetRetransmissionTimeout(std::chrono::milliseconds(10));
    
    std::string testData = "Retransmit test";
    auto packet = manager->PreparePacket(
        reinterpret_cast<const uint8_t*>(testData.c_str()),
        testData.length(),
        true
    );
    
    auto initialStats = manager->GetStats();
    
    // Wait for retransmission timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Call update to trigger retransmission check
    manager->Update();
    
    auto finalStats = manager->GetStats();
    
    // Should have attempted retransmission
    EXPECT_GT(finalStats.packetsRetransmitted, initialStats.packetsRetransmitted);
}