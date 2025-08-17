#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <cctype>

#include <net_message.h>
#include <net_packets.h>

using namespace x3::net;

const uint16_t DEFAULT_SERVER_PORT = 13337;

bool InitWinsock();
void ShutdownWinsock();

enum class ConnectionStatus
{
	Disconnected,
	Connecting,
	Connected,
	Failed
};

class Client
{
public:
	ConnectionStatus connectionStatus = ConnectionStatus::Disconnected;
	std::queue<x3::net::Packet*> receivedPackets;

	void Run(const char* ip, unsigned short port);
	void Stop();
	
	void SendPacket(Packet* message);
	void SendText(const std::string text);

private:
	bool m_bRunning = false;
	SOCKET m_socket = INVALID_SOCKET;
	sockaddr_in m_serverAddr;

	void Connect(const char* ip, unsigned short port);
	void PollIncomingMessages();
};