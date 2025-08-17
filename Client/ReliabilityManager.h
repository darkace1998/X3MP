#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <functional>

namespace x3 {
namespace net {

    using SequenceNumber = uint32_t;
    using AckNumber = uint32_t;
    
    // Reliable packet header (prepended to all packets)
    struct ReliableHeader {
        SequenceNumber sequence;
        AckNumber acknowledgment;
        uint16_t flags;
        uint16_t checksum;
    };
    
    // Flags for reliable header
    enum ReliableFlags : uint16_t {
        RELIABLE_FLAG_ACK = 0x01,      // This is an acknowledgment packet
        RELIABLE_FLAG_RELIABLE = 0x02,  // This packet requires acknowledgment
        RELIABLE_FLAG_RETRANSMIT = 0x04 // This is a retransmitted packet
    };
    
    // Pending packet for retransmission
    struct PendingPacket {
        std::vector<uint8_t> data;
        std::chrono::steady_clock::time_point lastSent;
        uint32_t retransmitCount;
        SequenceNumber sequence;
    };
    
    // Reliability manager for UDP connections
    class ReliabilityManager {
    public:
        using PacketCallback = std::function<void(const uint8_t* data, size_t size)>;
        
        ReliabilityManager();
        ~ReliabilityManager() = default;
        
        // Configure reliability parameters
        void SetRetransmissionTimeout(std::chrono::milliseconds timeout);
        void SetMaxRetransmissions(uint32_t maxRetransmissions);
        void SetPacketCallback(PacketCallback callback);
        
        // Send a packet (adds reliability header if needed)
        std::vector<uint8_t> PreparePacket(const uint8_t* data, size_t size, bool reliable = true);
        
        // Process received packet (handles ACKs and ordering)
        void ProcessReceivedPacket(const uint8_t* data, size_t size);
        
        // Update - call regularly to handle retransmissions
        void Update();
        
        // Send standalone ACK
        std::vector<uint8_t> CreateAck(SequenceNumber sequenceToAck);
        
        // Get statistics
        struct Stats {
            uint32_t packetsSent;
            uint32_t packetsReceived;
            uint32_t packetsRetransmitted;
            uint32_t duplicatesReceived;
            uint32_t outOfOrderReceived;
        };
        Stats GetStats() const { return m_stats; }
        
    private:
        SequenceNumber m_nextSequence;
        SequenceNumber m_lastReceivedSequence;
        AckNumber m_lastAcknowledged;
        
        std::unordered_map<SequenceNumber, PendingPacket> m_pendingPackets;
        std::unordered_map<SequenceNumber, std::vector<uint8_t>> m_receivedPackets;
        
        std::chrono::milliseconds m_retransmissionTimeout;
        uint32_t m_maxRetransmissions;
        
        PacketCallback m_packetCallback;
        Stats m_stats;
        
        uint16_t CalculateChecksum(const uint8_t* data, size_t size) const;
        bool IsSequenceNewer(SequenceNumber a, SequenceNumber b) const;
        void ProcessAcknowledgment(AckNumber ack);
        void DeliverOrderedPackets();
    };

}} // namespace x3::net