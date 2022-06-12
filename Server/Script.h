#pragma once

#include <memory>
#include "lua.hpp"
#include "Screen.h"
#include "Server.h"

int lua_CreateShip(lua_State *L); 
int lua_DeleteShip(lua_State* L);

class Script{
    private:
    lua_State *L;
    bool initialized = false;

    public:
    static std::shared_ptr<Script> Init(std::string path);
    void Start();
    void Stop();
    void call_OnPlayerConnect(int clientID);
    void call_OnConsoleCommand(std::string cmd);

    static void call_callback_OnPlayerConnect(int clientID);
    static void call_callback_OnConsoleCommand(std::string cmd);
};