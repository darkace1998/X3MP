#include "ReliabilityManager.h"
#include "Logger.h"
#include <algorithm>
#include <cstring>

namespace x3 {
namespace net {

    ReliabilityManager::ReliabilityManager()
        : m_nextSequence(1)
        , m_lastReceivedSequence(0)
        , m_lastAcknowledged(0)
        , m_retransmissionTimeout(std::chrono::milliseconds(100))
        , m_maxRetransmissions(5)
        , m_stats{}
    {
    }
    
    void ReliabilityManager::SetRetransmissionTimeout(std::chrono::milliseconds timeout) {
        m_retransmissionTimeout = timeout;
    }
    
    void ReliabilityManager::SetMaxRetransmissions(uint32_t maxRetransmissions) {
        m_maxRetransmissions = maxRetransmissions;
    }
    
    void ReliabilityManager::SetPacketCallback(PacketCallback callback) {
        m_packetCallback = callback;
    }
    
    std::vector<uint8_t> ReliabilityManager::PreparePacket(const uint8_t* data, size_t size, bool reliable) {
        std::vector<uint8_t> packet;
        packet.resize(sizeof(ReliableHeader) + size);
        
        ReliableHeader header{};
        header.sequence = m_nextSequence++;
        header.acknowledgment = m_lastReceivedSequence;
        header.flags = reliable ? RELIABLE_FLAG_RELIABLE : 0;
        header.checksum = CalculateChecksum(data, size);
        
        // Copy header and data
        std::memcpy(packet.data(), &header, sizeof(ReliableHeader));
        std::memcpy(packet.data() + sizeof(ReliableHeader), data, size);
        
        // Store for potential retransmission if reliable
        if (reliable) {
            PendingPacket pending;
            pending.data = packet;
            pending.lastSent = std::chrono::steady_clock::now();
            pending.retransmitCount = 0;
            pending.sequence = header.sequence;
            
            m_pendingPackets[header.sequence] = std::move(pending);
        }
        
        m_stats.packetsSent++;
        return packet;
    }
    
    void ReliabilityManager::ProcessReceivedPacket(const uint8_t* data, size_t size) {
        if (size < sizeof(ReliableHeader)) {
            LOG_WARNING("Received packet too small for reliable header");
            return;
        }
        
        ReliableHeader header;
        std::memcpy(&header, data, sizeof(ReliableHeader));
        
        const uint8_t* payload = data + sizeof(ReliableHeader);
        size_t payloadSize = size - sizeof(ReliableHeader);
        
        // Verify checksum
        uint16_t expectedChecksum = CalculateChecksum(payload, payloadSize);
        if (header.checksum != expectedChecksum) {
            LOG_WARNING("Packet checksum mismatch, dropping");
            return;
        }
        
        m_stats.packetsReceived++;
        
        // Process acknowledgment
        if (header.acknowledgment > m_lastAcknowledged) {
            ProcessAcknowledgment(header.acknowledgment);
        }
        
        // Handle ACK-only packets
        if (header.flags & RELIABLE_FLAG_ACK) {
            return; // Pure ACK packet, no payload to process
        }
        
        // Check if this is a duplicate
        if (m_receivedPackets.find(header.sequence) != m_receivedPackets.end()) {
            m_stats.duplicatesReceived++;
            return;
        }
        
        // Store the packet
        std::vector<uint8_t> payloadCopy(payload, payload + payloadSize);
        m_receivedPackets[header.sequence] = std::move(payloadCopy);
        
        // Update last received sequence if this is newer
        if (IsSequenceNewer(header.sequence, m_lastReceivedSequence)) {
            m_lastReceivedSequence = header.sequence;
        } else {
            m_stats.outOfOrderReceived++;
        }
        
        // Try to deliver ordered packets
        DeliverOrderedPackets();
    }
    
    void ReliabilityManager::Update() {
        auto now = std::chrono::steady_clock::now();
        
        // Check for packets that need retransmission
        for (auto& [seq, pending] : m_pendingPackets) {
            auto timeSinceLastSent = now - pending.lastSent;
            
            if (timeSinceLastSent >= m_retransmissionTimeout) {
                if (pending.retransmitCount < m_maxRetransmissions) {
                    // Mark as retransmission in header
                    ReliableHeader* header = reinterpret_cast<ReliableHeader*>(pending.data.data());
                    header->flags |= RELIABLE_FLAG_RETRANSMIT;
                    
                    pending.lastSent = now;
                    pending.retransmitCount++;
                    
                    m_stats.packetsRetransmitted++;
                    
                    LOG_DEBUG("Retransmitting packet " + std::to_string(seq) + 
                             " (attempt " + std::to_string(pending.retransmitCount) + ")");
                } else {
                    LOG_WARNING("Packet " + std::to_string(seq) + " exceeded max retransmissions, dropping");
                    // Note: In a real implementation, you might want to notify about connection failure
                }
            }
        }
    }
    
    std::vector<uint8_t> ReliabilityManager::CreateAck(SequenceNumber sequenceToAck) {
        std::vector<uint8_t> packet(sizeof(ReliableHeader));
        
        ReliableHeader header{};
        header.sequence = 0; // ACK packets don't need sequence numbers
        header.acknowledgment = sequenceToAck;
        header.flags = RELIABLE_FLAG_ACK;
        header.checksum = 0; // No payload to checksum
        
        std::memcpy(packet.data(), &header, sizeof(ReliableHeader));
        return packet;
    }
    
    uint16_t ReliabilityManager::CalculateChecksum(const uint8_t* data, size_t size) const {
        uint32_t sum = 0;
        
        // Sum all bytes
        for (size_t i = 0; i < size; ++i) {
            sum += data[i];
        }
        
        // Simple checksum: fold 32-bit sum to 16-bit
        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        
        return static_cast<uint16_t>(~sum);
    }
    
    bool ReliabilityManager::IsSequenceNewer(SequenceNumber a, SequenceNumber b) const {
        // Handle sequence number wraparound
        const uint32_t half_window = 0x80000000;
        return ((a > b) && (a - b <= half_window)) || 
               ((a < b) && (b - a > half_window));
    }
    
    void ReliabilityManager::ProcessAcknowledgment(AckNumber ack) {
        m_lastAcknowledged = ack;
        
        // Remove acknowledged packets from pending list
        auto it = m_pendingPackets.find(ack);
        if (it != m_pendingPackets.end()) {
            m_pendingPackets.erase(it);
        }
    }
    
    void ReliabilityManager::DeliverOrderedPackets() {
        if (!m_packetCallback) {
            return;
        }
        
        // Simple in-order delivery - deliver consecutive packets starting from the last delivered
        // Simple in-order delivery - deliver consecutive packets starting from the next expected sequence
        SequenceNumber expectedSequence = m_lastReceivedSequence + 1;
        
        auto it = m_receivedPackets.find(expectedSequence);
        while (it != m_receivedPackets.end()) {
            // Deliver this packet
            m_packetCallback(it->second.data(), it->second.size());
            
            // Remove from received packets
            m_receivedPackets.erase(it);
            
            // Look for next consecutive packet
            expectedSequence++;
            it = m_receivedPackets.find(expectedSequence);
        }
    }

}} // namespace x3::net