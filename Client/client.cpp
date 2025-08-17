#include "client.h"
#pragma warning( disable : 26812)

bool InitWinsock()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		LOG_ERROR("WSAStartup failed: " + std::to_string(iResult));
		return false;
	}
	return true;
}

void ShutdownWinsock()
{
	WSACleanup();
}

void Client::Stop() {
	m_bRunning = false;
	if (m_socket != INVALID_SOCKET) {
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

void Client::Run(const char* ip, unsigned short port)
{
	if (!InitWinsock()) {
		return;
	}

	// Initialize reliability manager
	m_reliabilityManager = std::make_unique<x3::net::ReliabilityManager>();
	m_reliabilityManager->SetPacketCallback(
		[this](const uint8_t* data, size_t size) {
			OnReliablePacketReceived(data, size);
		}
	);

	Connect(ip, port);
	if (connectionStatus == ConnectionStatus::Failed) {
		ShutdownWinsock();
		return;
	}

	m_bRunning = true;
	while (m_bRunning)
	{
		PollIncomingMessages();
		if (m_reliabilityManager) {
			m_reliabilityManager->Update();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	ShutdownWinsock();
}

void Client::Connect(const char* ip, unsigned short port)
{
	this->connectionStatus = ConnectionStatus::Connecting;

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET) {
		LOG_ERROR("Failed to create socket: " + std::to_string(WSAGetLastError()));
		this->connectionStatus = ConnectionStatus::Failed;
		return;
	}

	// Set non-blocking
	u_long mode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &mode) != 0) {
		LOG_ERROR("Failed to set non-blocking mode: " + std::to_string(WSAGetLastError()));
		closesocket(m_socket);
		this->connectionStatus = ConnectionStatus::Failed;
		return;
	}

	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &m_serverAddr.sin_addr);

	LOG_INFO("Client socket created. Ready to send to " + std::string(ip) + ":" + std::to_string(port));

	// Since UDP is connectionless, we'll consider ourselves "Connected" after the first packet is sent
	// and we receive a ConnectAcknowledge. For now, we are just "Connecting".
}

void Client::PollIncomingMessages()
{
	if (m_socket == INVALID_SOCKET) return;

	char recvbuf[1024];
	int recvbuflen = 1024;
	sockaddr_in fromAddr;
	int fromAddrSize = sizeof(fromAddr);

	while (true) {
		int iResult = recvfrom(m_socket, recvbuf, recvbuflen, 0, (SOCKADDR*)&fromAddr, &fromAddrSize);
		if (iResult == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				// No more data to read
				break;
			}
			else {
				LOG_ERROR("recvfrom failed: " + std::to_string(WSAGetLastError()));
				// Consider the connection failed
				this->connectionStatus = ConnectionStatus::Failed;
				Stop();
				break;
			}
		}

		if (iResult <= 0) {
			LOG_WARNING("Received empty or invalid packet");
			continue;
		}

		// Process through reliability manager
		if (m_reliabilityManager) {
			m_reliabilityManager->ProcessReceivedPacket(
				reinterpret_cast<const uint8_t*>(recvbuf), 
				static_cast<size_t>(iResult)
			);
		}
	}
}

void Client::OnReliablePacketReceived(const uint8_t* data, size_t size)
{
	if (size < sizeof(Packet)) {
		LOG_WARNING("Packet is malformed (too small)");
		return;
	}

	PacketType packetType = reinterpret_cast<const Packet*>(data)->type;
	std::unique_ptr<Packet> newPacket;

	switch (packetType)
	{
	case PacketType::ShipUpdate:
		if (size >= sizeof(ShipUpdate))
		{
			newPacket = std::make_unique<ShipUpdate>();
			std::memcpy(newPacket.get(), data, sizeof(ShipUpdate));
		}
		break;
	case PacketType::CreateShip:
		if (size >= sizeof(CreateShip))
		{
			newPacket = std::make_unique<CreateShip>();
			std::memcpy(newPacket.get(), data, sizeof(CreateShip));
		}
		break;
	case PacketType::DeleteShip:
		if (size >= sizeof(DeleteShip))
		{
			newPacket = std::make_unique<DeleteShip>();
			std::memcpy(newPacket.get(), data, sizeof(DeleteShip));
		}
		break;
	case PacketType::ConnectAcknowledge:
		if (size >= sizeof(ConnectAcknowledge))
		{
			newPacket = std::make_unique<ConnectAcknowledge>();
			std::memcpy(newPacket.get(), data, sizeof(ConnectAcknowledge));
			this->connectionStatus = ConnectionStatus::Connected;
			LOG_INFO("Connection Acknowledged by server!");
		}
		break;
	default:
		LOG_WARNING("Received unknown packet type: " + std::to_string(static_cast<int>(packetType)) + 
		           " (size: " + std::to_string(size) + " bytes)");
		break;
	}

	if (newPacket)
	{
		receivedPackets.push(std::move(newPacket));
	}
	else
	{
		LOG_ERROR("Packet is malformed (size mismatch or unknown type)");
	}
}

void Client::SendPacket(std::unique_ptr<Packet> message)
{
	if (m_socket == INVALID_SOCKET || !message) return;

	// Prepare packet with reliability layer
	if (m_reliabilityManager) {
		auto reliablePacket = m_reliabilityManager->PreparePacket(
			reinterpret_cast<const uint8_t*>(message.get()),
			message->size,
			true // Make packets reliable by default
		);

		int iResult = sendto(m_socket, 
						   reinterpret_cast<const char*>(reliablePacket.data()), 
						   static_cast<int>(reliablePacket.size()), 
						   0, 
						   reinterpret_cast<SOCKADDR*>(&m_serverAddr), 
						   sizeof(m_serverAddr));
		if (iResult == SOCKET_ERROR) {
			LOG_ERROR("sendto failed: " + std::to_string(WSAGetLastError()));
		}
	} else {
		// Fallback to direct send if reliability manager is not available
		int iResult = sendto(m_socket, 
						   reinterpret_cast<const char*>(message.get()), 
						   static_cast<int>(message->size), 
						   0, 
						   reinterpret_cast<SOCKADDR*>(&m_serverAddr), 
						   sizeof(m_serverAddr));
		if (iResult == SOCKET_ERROR) {
			LOG_ERROR("sendto failed: " + std::to_string(WSAGetLastError()));
		}
	}
}

void Client::SendText(const std::string text) {
	if (text.empty() || text.length() >= 512) {
		LOG_WARNING("Invalid text message length");
		return;
	}
	
	auto chatPacket = std::make_unique<x3::net::ChatMessage>();
	chatPacket->type = x3::net::PacketType::ChatMessage;
	chatPacket->size = sizeof(*chatPacket);
	
	// Set default white color
	chatPacket->A = 255;
	chatPacket->R = 255;
	chatPacket->G = 255;
	chatPacket->B = 255;
	
	// Copy message text with null termination
	strncpy_s(chatPacket->Message, sizeof(chatPacket->Message), text.c_str(), _TRUNCATE);
	
	SendPacket(std::move(chatPacket));
}


