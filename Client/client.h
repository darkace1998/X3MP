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
#include <memory>

#include <net_message.h>
#include <net_packets.h>
#include "Logger.h"
#include "ReliabilityManager.h"

using namespace x3::net;

const uint16 DEFAULT_SERVER_PORT = 13337;

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
	std::queue<std::unique_ptr<x3::net::Packet>> receivedPackets;

	void Run(const char* ip, unsigned short port);
	void Stop();
	
	void SendPacket(std::unique_ptr<Packet> message);
	void SendText(const std::string text);

private:
	bool m_bRunning = false;
	SOCKET m_socket = INVALID_SOCKET;
	sockaddr_in m_serverAddr;
	std::unique_ptr<x3::net::ReliabilityManager> m_reliabilityManager;

	void Connect(const char* ip, unsigned short port);
	void PollIncomingMessages();
	void OnReliablePacketReceived(const uint8_t* data, size_t size);
};