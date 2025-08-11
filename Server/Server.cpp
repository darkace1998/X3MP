#include "Server.h"
#include "Quaternion.h"

SteamNetworkingMicroseconds g_logTimeZero;

Server *Server::instance = 0;
std::unique_ptr<Server> ServerSingleton = std::unique_ptr<Server>(Server::getInstance());

static Server* s_pCallbackInstance;

bool g_bQuit = false;

void InitSteamDatagramConnectionSockets()
{
	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(nullptr, errMsg))
		Screen::LogError("GameNetworkingSockets_Init failed.");

	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

	// Fixed TODO: Reimplemented debug output function
	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, [](ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg) {
		Screen::Log(pszMsg);
	});
}

void ShutdownSteamDatagramConnectionSockets()
{
	// Give connections time to finish up.  This is an application layer protocol
	// here, it's not TCP.  Note that if you have an applicatistd::function<void(int)>is pending.
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	GameNetworkingSockets_Kill();
}

/////////////////////////////////////////////////////////////////////////////
//
// Server
//
/////////////////////////////////////////////////////////////////////////////

void Server::Init(std::shared_ptr<Universe> universe, std::function<void(int)> callback_OnPlayerConnect)
{
	this->universe = universe;
	this->callback_OnPlayerConnect = callback_OnPlayerConnect;
}

void Server::Run(uint16 nPort)
{
	// Select instance to use.  For now we'll always use the default.
	// But we could use SteamGameServerNetworkingSockets() on Steam.
	m_pInterface = SteamNetworkingSockets();

	// Start listening
	SteamNetworkingIPAddr serverLocalAddr;
	serverLocalAddr.Clear();
	serverLocalAddr.m_port = nPort;
	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
	m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
	if (m_hListenSock == k_HSteamListenSocket_Invalid)
		Screen::LogError("Failed to listen on port " + std::to_string(nPort));
	m_hPollGroup = m_pInterface->CreatePollGroup();
	if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
		Screen::LogError("Failed to listen on port " + std::to_string(nPort));

	// Fixed TODO: Properly convert port number to string to avoid garbage output
	Screen::Log("Server listening on port " + std::to_string(nPort));

	while (!g_bQuit)
	{
		PollIncomingMessages();
		PollConnectionStateChanges();
		std::string cmd = Screen::PollCommand();
		if(cmd == "exit")
			g_bQuit = true;
		if (cmd.rfind("say ", 0) == 1)
		{
			x3::net::ChatMessage message;
			cmd = cmd.erase(0, 4).insert(0, "Server: ");
			memcpy(message.Message, cmd.c_str(), cmd.length() - 4);
			SendPacketToAllClients(&message);
			continue;
		}
		if (!cmd.empty())
			Script::call_callback_OnConsoleCommand(cmd);
		
		// Fixed TODO: Improved sleep timing - use shorter sleep for better responsiveness
		// and longer sleep when no activity to reduce CPU usage
		if (cmd.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	// Close all the connections
	Screen::Log("Closing connections...\n");
	for (auto it : m_mapClients)
	{
		// Send them one more goodbye message.  Note that we also have the
		// connection close reason as a place to send final data.  However,
		// that's usually best left for more diagnostic/debug text not actual
		// protocol strings.
		//SendStringToClient(it.first, "Server is shutting down.  Goodbye.");

		// Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
		// to flush this out and close gracefully.
		m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
	}
	m_mapClients.clear();

	m_pInterface->CloseListenSocket(m_hListenSock);
	m_hListenSock = k_HSteamListenSocket_Invalid;

	m_pInterface->DestroyPollGroup(m_hPollGroup);
	m_hPollGroup = k_HSteamNetPollGroup_Invalid;
}

void Server::SendPacketToClient(HSteamNetConnection conn, x3::net::Packet* packet)
{
	m_pInterface->SendMessageToConnection(conn, packet, packet->size, k_nSteamNetworkingSend_Reliable, nullptr);
}

void Server::SendStringToClient(HSteamNetConnection conn, const char* str)
{
	m_pInterface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
}

void Server::SendPacketToAllClients(x3::net::Packet* packet, HSteamNetConnection except)
{
	for (auto& c : m_mapClients)
	{
		if (c.first != except)
			SendPacketToClient(c.first, packet);
	}
}

void Server::SendStringToAllClients(const char* str, HSteamNetConnection except)
{
	for (auto& c : m_mapClients)
	{
		if (c.first != except)
			SendStringToClient(c.first, str);
	}
}

void Server::PollIncomingMessages()
{
	while (!g_bQuit)
	{
		ISteamNetworkingMessage* pIncomingMsg = nullptr;
		int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMsg, 1);
		if (numMsgs == 0)
			break;
		if (numMsgs < 0)
			Screen::LogError("Error checking for messages");
		assert(numMsgs == 1 && pIncomingMsg);
		auto itClient = m_mapClients.find(pIncomingMsg->m_conn);
		assert(itClient != m_mapClients.end());

		x3::net::Packet* packet = (x3::net::Packet*)pIncomingMsg->m_pData;

		//Screen::LogDebug(std::string("Package: ") + std::to_string((int)packet->type));

		if (packet->type == x3::net::PacketType::ShipUpdate)
		{
			//Screen::LogDebug("Ship update.");
			x3::net::ShipUpdate updatePacket;
			memcpy(&updatePacket, pIncomingMsg->m_pData, sizeof(x3::net::ShipUpdate));

			if (m_mapClients[pIncomingMsg->m_conn].clientID == (*universe->entities)[updatePacket.ShipID]->NetOwnerID)
			{
				(*universe->entities)[updatePacket.ShipID]->PosX = updatePacket.PosX;
				(*universe->entities)[updatePacket.ShipID]->PosY = updatePacket.PosY;
				(*universe->entities)[updatePacket.ShipID]->PosZ = updatePacket.PosZ;
				(*universe->entities)[updatePacket.ShipID]->RotX = updatePacket.RotX;
				(*universe->entities)[updatePacket.ShipID]->RotY = updatePacket.RotY;
				(*universe->entities)[updatePacket.ShipID]->RotZ = updatePacket.RotZ;
				(*universe->entities)[updatePacket.ShipID]->RotW = updatePacket.RotW;
				(*universe->entities)[updatePacket.ShipID]->UpX = updatePacket.UpX;
				(*universe->entities)[updatePacket.ShipID]->UpY = updatePacket.UpY;
				(*universe->entities)[updatePacket.ShipID]->UpZ = updatePacket.UpZ;
				(*universe->entities)[updatePacket.ShipID]->LookAtX = updatePacket.LookAtX;
				(*universe->entities)[updatePacket.ShipID]->LookAtY = updatePacket.LookAtY;
				(*universe->entities)[updatePacket.ShipID]->LookAtZ = updatePacket.LookAtZ;
			}
			else
			{
				std::stringstream stream;
				stream << "Ignoring packet for ship " << updatePacket.ShipID << ". NetOwner missmatch! Owner is " << (*universe->entities)[updatePacket.ShipID]->NetOwnerID << " but packet was sent by " << m_mapClients[pIncomingMsg->m_conn].clientID;
				Screen::Log(stream.str());
			}

			SendPacketToAllClients(&updatePacket, pIncomingMsg->m_conn);
		}
			
		if (packet->type == x3::net::PacketType::Connect)
		{
			//Screen::LogDebug("Connect package.");
			x3::net::Connect connectPacket;
			memcpy(&connectPacket, pIncomingMsg->m_pData, sizeof(x3::net::Connect));

			m_mapClients[pIncomingMsg->m_conn].clientID = lastClientID;

			x3::net::ConnectAcknowledge acknowledge;
			acknowledge.ClientID = lastClientID;
			acknowledge.size = sizeof(x3::net::ConnectAcknowledge);
			acknowledge.type = x3::net::PacketType::ConnectAcknowledge;

			acknowledge.ShipID = CreateShip(connectPacket.Model);

			SendPacketToClient(pIncomingMsg->m_conn, &acknowledge);

			for (size_t i = 0; i < 65535; i++)
			{
				if ((*universe->entities).at(i) == nullptr || i == acknowledge.ShipID)
					continue;

				x3::net::CreateShip packet;
				packet.ShipID = i;
				packet.Model = (*universe->entities)[i]->Model;
				packet.type = x3::net::PacketType::CreateShip;
				packet.size = sizeof(x3::net::CreateShip);
				packet.PosX = (*universe->entities)[i]->PosX;
				packet.PosY = (*universe->entities)[i]->PosY;
				packet.PosZ = (*universe->entities)[i]->PosZ;
				packet.RotX = (*universe->entities)[i]->RotX;
				packet.RotY = (*universe->entities)[i]->RotY;
				packet.RotZ = (*universe->entities)[i]->RotZ;
				packet.RotW = (*universe->entities)[i]->RotW;
				packet.UpX = (*universe->entities)[i]->UpX;
				packet.UpY = (*universe->entities)[i]->UpY;
				packet.UpZ = (*universe->entities)[i]->UpZ;
				packet.LookAtX = (*universe->entities)[i]->LookAtX;
				packet.LookAtY = (*universe->entities)[i]->LookAtY;
				packet.LookAtZ = (*universe->entities)[i]->LookAtZ;
				SendPacketToClient(pIncomingMsg->m_conn, &packet);
			}

			(*universe->entities).at(acknowledge.ShipID)->NetOwnerID = acknowledge.ClientID;

			lastClientID++;

			Script::call_callback_OnPlayerConnect(m_mapClients[pIncomingMsg->m_conn].clientID);
		}
		pIncomingMsg->Release();
	}
}

size_t Server::CreateShip(int32_t model)
{
	for (size_t i = 0; i < 65535; i++)
	{
		if ((*universe->entities).at(i) != nullptr)
			continue;
		(*universe->entities)[i] = std::make_shared<x3::net::Entity>();
		(*universe->entities)[i]->Model = model;
		(*universe->entities)[i]->NetOwnerID = -1;
		(*universe->entities)[i]->Owner = -1;

		x3::net::CreateShip packet;
		packet.ShipID = i;
		packet.Model = model;
		packet.type = x3::net::PacketType::CreateShip;
		packet.size = sizeof(x3::net::CreateShip);
		/*Quaternion rot = ToQuaternion(0, 0, 0);
		Quaternion up = ToQuaternion(0, 1, 0);
		packet.PosX = 0;
		packet.PosY = 0;
		packet.PosZ = 0;
		packet.UpX = up.x;
		packet.UpY = up.y;
		packet.UpZ = up.z;
		packet.UpW = up.w;
		packet.RotX = rot.x;
		packet.RotY = rot.y;
		packet.RotZ = rot.z;
		packet.RotW = rot.w;
		*/
		packet.UpX = 0;
		packet.UpY = 0;
		packet.UpZ = 0;
		packet.UpW = 0;
		packet.RotX = 0;
		packet.RotY = 0;
		packet.RotZ = 0;
		packet.RotW = 0;
		SendPacketToAllClients(&packet);
		return i;
	}
	return -1;
}

void Server::DeleteShip(size_t id)
{
	if ((*universe->entities).at(id) == nullptr)
		return;
	(*universe->entities)[id] = nullptr;

	x3::net::DeleteShip packet;
	packet.ShipID = id;
	packet.type = x3::net::PacketType::DeleteShip;
	packet.size = sizeof(x3::net::DeleteShip);
	SendPacketToAllClients(&packet);
}

/*void Server::PollLocalUserInput()
{
	std::string cmd;
	while (!g_bQuit && LocalUserInput_GetNext(cmd))
	{
		std::istringstream iss(cmd);
		std::vector<std::string> commandArray(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

		if (commandArray[0].compare("/quit") == 0)
		{
			g_bQuit = true;
			Screen::Log("Shutting down server");
			break;
		}

		if (commandArray[0].compare("/discoverer") == 0)
		{
			for (size_t i = 0; i < 65535; i++)
			{
				if (entities.at(i) != nullptr)
					continue;
				entities[i] = std::make_shared<x3::net::Entity>();
				entities[i]->Model = 0x16;
				entities[i]->NetOwnerID = -1;
				entities[i]->Owner = -1;

				x3::net::CreateShip packet;
				packet.ShipID = i;
				packet.Model = 0x16;
				packet.type = x3::net::PacketType::CreateShip;
				packet.size = sizeof(x3::net::CreateShip);
				packet.PosX = 0;
				packet.PosY = 0;
				packet.PosZ = 0;
				SendPacketToAllClients(&packet);
				break;
			}
			break;
		}

		//Broken
		
		if (commandArray[0].compare("/sun") == 0)
		{
			for (size_t i = 0; i < 65535; i++)
			{
				if (stars.at(i) != nullptr)
					continue;
				stars[i] = std::make_shared<x3::net::Entity>();
				stars[i]->Model = 0x1;

				x3::net::CreateStar packet;
				packet.StarID = i;
				packet.Model = 0x1;
				packet.type = x3::net::PacketType::CreateStar;
				packet.size = sizeof(x3::net::CreateStar);
				packet.PosX = 100000;
				packet.PosY = 100000;
				packet.PosZ = 100000;
				SendPacketToAllClients(&packet);
				break;
			}
			break;
		}

		Screen::Log("Help: /quit /discoverer /sun");
	}
}*/

void Server::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	// What's the state of the connection?
	switch (pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_None:
		// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
		break;

	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		// Ignore if they were not previously connected.  (If they disconnected
		// before we accepted the connection.)
		if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
		{

			// Locate the client.  Note that it should have been found, because this
			// is the only codepath where we remove clients (except on shutdown),
			// and connection change callbacks are dispatched in queue order.
			auto itClient = m_mapClients.find(pInfo->m_hConn);
			assert(itClient != m_mapClients.end());

			// Select appropriate log messages
			const char* pszDebugLogAction;
			if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
			{
				pszDebugLogAction = "problem detected locally";
				//sprintf_s(temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.m_sNick.c_str(), pInfo->m_info.m_szEndDebug);
				Screen::Log(std::string("Alas, ") + itClient->second.m_sNick.c_str() + std::string("hath fallen into shadow. (") + pInfo->m_info.m_szEndDebug + std::string(")"));
			}
			else
			{
				// Note that here we could check the reason code to see if
				// it was a "usual" connection or an "unusual" one.
				pszDebugLogAction = "closed by peer";
				//sprintf_s(temp, "%s hath departed", itClient->second.m_sNick.c_str());
				Screen::Log(std::string() + itClient->second.m_sNick.c_str() + " has departed.");
			}

			// Spew something to our own log.  Note that because we put their nick
			// as the connection description, it will show up, along with their
			// transport-specific data (e.g. their IP address)
			std::stringstream stream;
			stream << "Connection" << pInfo->m_info.m_szConnectionDescription << pszDebugLogAction << "reason: " << pInfo->m_info.m_eEndReason << " " << pInfo->m_info.m_szEndDebug;
			Screen::Log(stream.str());

			m_mapClients.erase(itClient);

			// Send a message so everybody else knows what happened
			//SendStringToAllClients(temp);
		}
		else
		{
			assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
		}

		// Clean up the connection.  This is important!
		// The connection is "closed" in the network sense, but
		// it has not been destroyed.  We must close it on our end, too
		// to finish up.  The reason information do not matter in this case,
		// and we cannot linger because it's already closed on the other end,
		// so we just pass 0's.
		m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
		break;
	}

	case k_ESteamNetworkingConnectionState_Connecting:
	{
		// This must be a new connection
		assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

		Screen::Log(std::string("Connection request from ") + pInfo->m_info.m_szConnectionDescription);

		// A client is attempting to connect
		// Try to accept the connection.
		if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
		{
			// This could fail.  If the remote host tried to connect, but then
			// disconnected, the connection may already be half closed.  Just
			// destroy whatever we have on our side.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			Screen::Log("Can't accept connection.  (It was already closed?)");
			break;
		}

		// Assign the poll group
		if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup))
		{
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			Screen::Log("Failed to set poll group?");
			break;
		}

		// Add them to the client list, using std::map wacky syntax
		m_mapClients[pInfo->m_hConn];
		break;
	}

	case k_ESteamNetworkingConnectionState_Connected:
		// We will get a callback immediately after accepting the connection.
		// Since we are the server, we can ignore this, it's not news to us.
		break;

	default:
		// Silences -Wswitch
		break;
	}
}

void Server::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
}

void Server::PollConnectionStateChanges()
{
	s_pCallbackInstance = this;
	m_pInterface->RunCallbacks();
}
