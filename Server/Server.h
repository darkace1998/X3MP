#pragma once

#include <array>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <map>
#include <cctype>
#include <sstream>
#include <vector>
#include <iterator>
#include <functional>

#ifdef WIN32
#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>
#else
#include "../../SDKs/GameNetworkingSockets/include/steam/steamnetworkingsockets.h"
#include "../../SDKs/GameNetworkingSockets/include/steam/isteamnetworkingutils.h"
#endif
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <GameNetworkingSockets/steam/steam_api.h>
#endif

#include "Universe.h"
#include "Screen.h"
#include <net_message.h>
#include <net_packets.h>
#include <net_entity.h>
#include "Script.h"



void InitSteamDatagramConnectionSockets();
void ShutdownSteamDatagramConnectionSockets();

class Server
{
public:
	void Init(std::shared_ptr<Universe> universe, std::function<void(int)> callback_OnPlayerConnect);
	void Run(uint16 nPort);
	size_t CreateShip(int32_t model);
	void DeleteShip(size_t id);

	std::function<void(int)> callback_OnPlayerConnect;

	static Server *instance;
	static Server* getInstance(){
		if(!instance) instance = new Server;
		return instance;
	}

private:
	Server() {	}
	std::shared_ptr<Universe> universe;
	HSteamListenSocket m_hListenSock = 0;
	HSteamNetPollGroup m_hPollGroup= 0;
	ISteamNetworkingSockets* m_pInterface = 0;

	struct Client_t
	{
		std::string m_sNick;
		int32_t clientID = -1;
	};

	std::map< HSteamNetConnection, Client_t > m_mapClients;
	int32_t lastClientID = 0; 

	void SendPacketToClient(HSteamNetConnection conn, x3::net::Packet* packet);
	void SendStringToClient(HSteamNetConnection conn, const char* str);
	void SendPacketToAllClients(x3::net::Packet* packet, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
	void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);

	void PollIncomingMessages();

	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);

	void PollConnectionStateChanges();

protected:
	std::shared_ptr<Universe> GetUniverse() const { return universe; }
};

extern std::unique_ptr<Server> ServerSingleton;