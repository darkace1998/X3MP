#include "client.h"
#pragma warning( disable : 26812)

Client* s_pCallbackInstance = nullptr;


void InitSteamDatagramConnectionSockets()
{
	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(nullptr, errMsg))
		std::cout << "GameNetworkingSockets_Init failed." << errMsg << std::endl;

	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

void ShutdownSteamDatagramConnectionSockets()
{
	// Give connections time to finish up.  This is an application layer protocol
	// here, it's not TCP.  Note that if you have an application and you need to be
	// more sure about cleanup, you won't be able to do this.  You will need to send
	// a message and then either wait for the peer to close the connection, or
	// you can pool the connection to see if any reliable data is pending.
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	GameNetworkingSockets_Kill();
}

void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
	SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
	std::cout << "[DBG] " << pszMsg << std::endl;
}

void Client::Stop() {
	g_bQuit = true;
}

bool Client::CheckConnectionTimeout() {
	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - connectionStartTime);
	return elapsed.count() >= CONNECTION_TIMEOUT_MS;
}

void Client::Run(const char* ip, unsigned short port)
	{
	this->isConnected = false;
	this->hasConnectionFailed = false;
	this->hasTimedOut = false;
	this->connectionStartTime = std::chrono::steady_clock::now();
	
		SteamNetworkingIPAddr serverAddr; serverAddr.Clear();
/*
#ifdef _DEBUG
		std::cout << "Please insert the address of the gameserver:" << std::endl;
		std::string ip;
		std::getline(std::cin, ip);
		std::cin.ignore();
		serverAddr.ParseString(ip.c_str());
#else*/
		serverAddr.ParseString(ip);
//#endif
		serverAddr.m_port = port;

		// Create client and server sockets
		InitSteamDatagramConnectionSockets();

		// Select instance to use.  For now we'll always use the default.
		m_pInterface = SteamNetworkingSockets();

		// Start connecting
		char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
		serverAddr.ToString(szAddr, sizeof(szAddr), true);
		std::cout << "[INF] Connecting to server at " << szAddr << std::endl;
		SteamNetworkingConfigValue_t opt;
		opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
		m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
		if (m_hConnection == k_HSteamNetConnection_Invalid)
			std::cout << "[ERR] Failed to create connection" << std::endl;

		while (!g_bQuit)
		{
			PollIncomingMessages();
			PollConnectionStateChanges();
			
			// Check for connection timeout
			if (!isConnected && !hasConnectionFailed && CheckConnectionTimeout()) {
				hasTimedOut = true;
				std::cout << "[ERR] Connection timeout after " << CONNECTION_TIMEOUT_MS/1000 << " seconds" << std::endl;
				Stop();
				break;
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	HSteamNetConnection m_hConnection;
	ISteamNetworkingSockets* m_pInterface;

	void Client::PollIncomingMessages()
	{
		while (!g_bQuit)
		{
			ISteamNetworkingMessage* pIncomingMsg = nullptr;
			int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
			if (numMsgs == 0)
				break;
			if (numMsgs < 0)
			{
				std::cout << "[ERR] Error checking for messages" << std::endl;
			}

			Packet* packet = new Packet();
			if (pIncomingMsg->GetSize() < sizeof(Packet))
			{
				std::cout << "[ERR] Packet is malformed." << std::endl;
				pIncomingMsg->Release();
				break;
			}
			memcpy(packet, pIncomingMsg->m_pData, sizeof(Packet));

			if (packet->type == PacketType::ShipUpdate)
			{
				ShipUpdate* updatePacket = new ShipUpdate();
				memcpy(updatePacket, pIncomingMsg->m_pData, sizeof(ShipUpdate));
				receivedPackets.push(updatePacket);
			}
			if (packet->type == PacketType::CreateShip)
			{
				CreateShip* createPacket = new CreateShip();
				memcpy(createPacket, pIncomingMsg->m_pData, sizeof(CreateShip));
				receivedPackets.push(createPacket);
			}
			if (packet->type == PacketType::DeleteShip)
			{
				DeleteShip* deletePacket = new DeleteShip();
				memcpy(deletePacket, pIncomingMsg->m_pData, sizeof(DeleteShip));
				receivedPackets.push(deletePacket);
			}
			if (packet->type == PacketType::ConnectAcknowledge)
			{
				ConnectAcknowledge* conAckPacket = new ConnectAcknowledge();
				memcpy(conAckPacket, pIncomingMsg->m_pData, sizeof(ConnectAcknowledge));
				receivedPackets.push(conAckPacket);
			}

			// We don't need this anymore.
			pIncomingMsg->Release();
		}
	}

	void Client::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
	{
		assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

		// What's the state of the connection?
		switch (pInfo->m_info.m_eState)
		{
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			g_bQuit = true;
			hasConnectionFailed = true;

			// Print an appropriate message
			if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
			{
				// Note: we could distinguish between a timeout, a rejected connection,
				// or some other transport problem.
				std::cout << "We sought the remote host, yet our efforts were met with defeat.  " << pInfo->m_info.m_szEndDebug << std::endl;
			}
			else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
			{
				std::cout << "Alas, troubles beset us; we have lost contact with the host.  " << pInfo->m_info.m_szEndDebug << std::endl;
			}
			else
			{
				// NOTE: We could check the reason code for a normal disconnection
				std::cout << "The host hath bidden us farewell.  " << pInfo->m_info.m_szEndDebug << std::endl;
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			m_hConnection = k_HSteamNetConnection_Invalid;
			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
			// We will get this callback when we start connecting.
			// We can ignore this.
			break;

		case k_ESteamNetworkingConnectionState_Connected:
			std::cout << "[INF] Connected to server!" << std::endl;
			this->isConnected = true;
			break;

		default:
			// Silences -Wswitch
			break;
		}
	}

	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
	{
		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}

	void Client::PollConnectionStateChanges()
	{
		s_pCallbackInstance = this;
		m_pInterface->RunCallbacks();
	}

	void Client::SendPacket(Packet* message, const int transferType)
	{
		/*void* msg = malloc(message->size() + sizeof(message_header<PacketType>));
		memcpy(msg, message, sizeof(message_header<PacketType>));
		unsigned char* bytePtr = reinterpret_cast<unsigned char*>(msg);
		bytePtr += sizeof(message_header<PacketType>);
		memcpy(bytePtr, message->body.data(), message->size());
		m_pInterface->SendMessageToConnection(m_hConnection, msg, message->size() + sizeof(message_header<PacketType>), transferType, nullptr);*/

		m_pInterface->SendMessageToConnection(m_hConnection, message, message->size, transferType, nullptr);
	}

	void Client::SendText(const std::string text, const int transferType) {
		if (m_hConnection == k_HSteamNetConnection_Invalid) {
			return;
		}

		m_pInterface->SendMessageToConnection(m_hConnection, text.c_str(), (uint32)text.length(), transferType, nullptr);
	}


