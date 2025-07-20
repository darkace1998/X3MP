#include <array>
#include <iostream>

#include "Server.h"
#include "Screen.h"
#include "Script.h"
#include "Universe.h"
#include "Config.h"

#define DEBUG

int main(int argc, const char* argv[])
{
	// Initialize configuration
	Config config("config.json");
	if (!config.load()) {
		// Create default config file if it doesn't exist
		config.save();
		std::cout << "Created default configuration file: config.json" << std::endl;
	}

	Screen::Start();

	Screen::Log("==================================================================");
    Screen::Log("      X3MP started...");
    Screen::Log("==================================================================");
	
	// Print configuration
	config.print();

	std::shared_ptr<Universe> universe = std::make_shared<Universe>();

	// Create server socket
	InitSteamDatagramConnectionSockets();

	Screen::Log(" Loading resources...");
	std::string scriptFile = config.getLuaScript();
	Screen::Log((" Loading script: " + scriptFile + "...").c_str(), false);
	std::shared_ptr<Script> script = Script::Init(scriptFile);
	ServerSingleton->Init(universe, Script::call_callback_OnPlayerConnect);

	if(script != nullptr)
	{
		Screen::Log(" done");
	}
	else
	{
		Screen::Log(" Error loading script");
	}

	script->Start();

	uint16 nPort = config.getServerPort();
	Screen::Log((" Server will start on port " + std::to_string(nPort)).c_str());
	
	SteamNetworkingIPAddr addrServer; 
	addrServer.Clear();

	ServerSingleton->Run(nPort);

	ShutdownSteamDatagramConnectionSockets();

	script->Stop();

	Screen::Stop();
	
	return 0;
}