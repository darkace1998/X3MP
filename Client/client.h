#pragma once

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

#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <GameNetworkingSockets/steam/steam_api.h>
#endif
#include <net_message.h>
#include <net_packets.h>

using namespace x3::net;

const uint16 DEFAULT_SERVER_PORT = 13337;

void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg);
void InitSteamDatagramConnectionSockets();
void ShutdownSteamDatagramConnectionSockets();
void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);

static SteamNetworkingMicroseconds g_logTimeZero;

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
	void Reconnect();
	
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

	void SendPacket(Packet* message, const int transferType = k_nSteamNetworkingSend_Reliable);
	void SendText(const std::string text, const int transferType = k_nSteamNetworkingSend_Reliable);

private:
	bool m_bRunning = false;
	std::string m_serverIp;
	unsigned short m_serverPort;
	HSteamNetConnection m_hConnection = 0;
	ISteamNetworkingSockets* m_pInterface = 0;

	void Connect();
	void PollIncomingMessages();
	void PollConnectionStateChanges();
};