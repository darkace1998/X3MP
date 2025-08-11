#include "client.h"
#pragma warning( disable : 26812)

bool InitWinsock()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "[ERR] WSAStartup failed: " << iResult << std::endl;
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

	Connect(ip, port);
	if (connectionStatus == ConnectionStatus::Failed) {
		ShutdownWinsock();
		return;
	}

	m_bRunning = true;
	while (m_bRunning)
	{
		PollIncomingMessages();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	ShutdownWinsock();
}

void Client::Connect(const char* ip, unsigned short port)
{
	this->connectionStatus = ConnectionStatus::Connecting;

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET) {
		std::cout << "[ERR] Failed to create socket: " << WSAGetLastError() << std::endl;
		this->connectionStatus = ConnectionStatus::Failed;
		return;
	}

	// Set non-blocking
	u_long mode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &mode) != 0) {
		std::cout << "[ERR] Failed to set non-blocking mode: " << WSAGetLastError() << std::endl;
		closesocket(m_socket);
		this->connectionStatus = ConnectionStatus::Failed;
		return;
	}

	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &m_serverAddr.sin_addr);

	std::cout << "[INF] Client socket created. Ready to send to " << ip << ":" << port << std::endl;

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
				std::cout << "[ERR] recvfrom failed: " << WSAGetLastError() << std::endl;
				// Consider the connection failed
				this->connectionStatus = ConnectionStatus::Failed;
				Stop();
				break;
			}
		}

		if (iResult < sizeof(Packet)) {
			std::cout << "[ERR] Packet is malformed (too small)." << std::endl;
			continue;
		}

		PacketType packetType = ((Packet*)recvbuf)->type;
		Packet* newPacket = nullptr;

		switch (packetType)
		{
		case PacketType::ShipUpdate:
			if (iResult >= sizeof(ShipUpdate))
			{
				newPacket = new ShipUpdate();
				memcpy(newPacket, recvbuf, sizeof(ShipUpdate));
			}
			break;
		case PacketType::CreateShip:
			if (iResult >= sizeof(CreateShip))
			{
				newPacket = new CreateShip();
				memcpy(newPacket, recvbuf, sizeof(CreateShip));
			}
			break;
		case PacketType::DeleteShip:
			if (iResult >= sizeof(DeleteShip))
			{
				newPacket = new DeleteShip();
				memcpy(newPacket, recvbuf, sizeof(DeleteShip));
			}
			break;
		case PacketType::ConnectAcknowledge:
			if (iResult >= sizeof(ConnectAcknowledge))
			{
				newPacket = new ConnectAcknowledge();
				memcpy(newPacket, recvbuf, sizeof(ConnectAcknowledge));
				this->connectionStatus = ConnectionStatus::Connected;
				std::cout << "[INF] Connection Acknowledged by server!" << std::endl;
			}
			break;
		default:
			std::cout << "[WARN] Received unknown packet type: " << (int)packetType 
					  << " from " << inet_ntoa(fromAddr.sin_addr) 
					  << ":" << ntohs(fromAddr.sin_port) 
					  << " (size: " << iResult << " bytes)" << std::endl;
			break;
		}

		if (newPacket)
		{
			receivedPackets.push(newPacket);
		}
		else
		{
			std::cout << "[ERR] Packet is malformed (size mismatch or unknown type)." << std::endl;
		}
	}
}

void Client::SendPacket(Packet* message)
{
	if (m_socket == INVALID_SOCKET) return;

	int iResult = sendto(m_socket, (const char*)message, message->size, 0, (SOCKADDR*)&m_serverAddr, sizeof(m_serverAddr));
	if (iResult == SOCKET_ERROR) {
		std::cout << "[ERR] sendto failed: " << WSAGetLastError() << std::endl;
	}
}

void Client::SendText(const std::string text) {
	if (text.empty() || text.length() >= 512) {
		std::cout << "[WARN] Invalid text message length" << std::endl;
		return;
	}
	
	x3::net::ChatMessage chatPacket;
	chatPacket.type = x3::net::PacketType::ChatMessage;
	chatPacket.size = sizeof(chatPacket);
	
	// Set default white color
	chatPacket.A = 255;
	chatPacket.R = 255;
	chatPacket.G = 255;
	chatPacket.B = 255;
	
	// Copy message text with null termination
	strncpy_s(chatPacket.Message, sizeof(chatPacket.Message), text.c_str(), _TRUNCATE);
	
	SendPacket(&chatPacket);
}


