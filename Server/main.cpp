#include <array>
#include <iostream>

#include "Server.h"
#include "Screen.h"
#include "Script.h"
#include "Universe.h"

#define DEBUG

int main(int argc, const char* argv[])
{
	Screen::Start();

	Screen::Log("==================================================================");
    Screen::Log("      X3MP started...");
    Screen::Log("==================================================================");

	std::shared_ptr<Universe> universe = std::make_shared<Universe>();

	// Create server socket
	InitSteamDatagramConnectionSockets();

	Screen::Log(" Loading resources...");
	Screen::Log(" Test resource from luascript.lua...", false);
	std::shared_ptr<Script> script = Script::Init(std::string("luascript.lua"));
	ServerSingleton->Init(universe, Script::call_callback_OnPlayerConnect);

	if(script != nullptr)
	{
		Screen::Log(" done");
	}
	else
	{
		Screen::Log(" Error");
	}

	script->Start();

	uint16 nPort = 13337;
	SteamNetworkingIPAddr addrServer; 
	addrServer.Clear();

	ServerSingleton->Run(nPort);

	ShutdownSteamDatagramConnectionSockets();

	script->Stop();

	Screen::Stop();
}