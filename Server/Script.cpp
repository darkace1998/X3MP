#include "Script.h"

std::vector<std::shared_ptr<Script>> scripts = std::vector<std::shared_ptr<Script>>();

void Script::call_callback_OnPlayerConnect(int clientID)
{
    for(const auto& value: scripts)
    {
        value->call_OnPlayerConnect(clientID);
    }
}

void Script::call_callback_OnConsoleCommand(std::string cmd)
{
    for (const auto& value : scripts)
    {
        value->call_OnConsoleCommand(cmd);
    }
}

static int lua_print(lua_State* L) {
    int nargs = lua_gettop(L);

    for (int i=1; i <= nargs; i++) {
        if (lua_isstring(L, i)) {
            /* Pop the next arg using lua_tostring(L, i) and do your print */
            //std::cout << "[LUA]" << lua_tostring(L, i) << std::endl;
            Screen::Log(lua_tostring(L, i));
        }
    }

    return 0;
}

static const struct luaL_Reg printlib [] = {
    {"print", lua_print},
    {NULL, NULL} /* end of array */
};

extern int luaopen_luamylib(lua_State *L)
{
    lua_getglobal(L, "_G");
    luaL_setfuncs(L, printlib, 0);
    lua_pop(L, 1);
    return 0;
}

std::shared_ptr<Script> Script::Init(std::string path)
{
    std::shared_ptr<Script> script = std::make_shared<Script>();
    scripts.push_back(script);
    script->L = luaL_newstate();
    luaL_openlibs(script->L);
    luaopen_luamylib(script->L);
    lua_register(script->L, "createShip", lua_CreateShip);
    lua_register(script->L, "deleteShip", lua_DeleteShip);
    if (luaL_dofile(script->L, path.c_str())) {
        Screen::LogError(lua_tostring(script->L, -1));
        return nullptr;
    }
    script->initialized = true;
    return script;
}

void Script::Start()
{
    if(!initialized)
        return;
    lua_getglobal(L, "onScriptStart");
    int x = lua_pcall(L, 0, 1, 0);
    if(x != 0)
    {
        Screen::LogError("There was an error during function execution. Error code: " + std::to_string(x));
    }
    lua_pop(L,1);
}

void Script::Stop()
{
    if(!initialized)
        return;
    lua_getglobal(L, "onScriptStop");
    int x = lua_pcall(L, 0, 1, 0);
    if(x != 0)
    {
        Screen::LogError("There was an error during function execution. Error code: " + std::to_string(x));
    }
    lua_pop(L,1);
    lua_close(L);
}

void Script::call_OnPlayerConnect(int clientID)
{
    if(!initialized)
        return;
    lua_getglobal(L, "onPlayerConnect");
    lua_pushnumber(L, clientID);
    int x = lua_pcall(L, 1, 1, 0);
    if(x != 0)
    {
        Screen::LogError("There was an error during function execution. Error code: " + std::to_string(x));
    }
    lua_pop(L,1);
}

void Script::call_OnConsoleCommand(std::string cmd)
{
    if (!initialized)
        return;
    lua_getglobal(L, "onConsoleCommand");
    lua_pushstring(L, cmd.c_str());
    int x = lua_pcall(L, 1, 1, 0);
    if (x != 0)
    {
        Screen::LogError("There was an error during function execution. Error code: " + std::to_string(x));
    }
    lua_pop(L, 1);
}

int lua_CreateShip(lua_State* L)
{
    int32_t model = (int32_t)lua_tonumber(L, 1);
    size_t id = ServerSingleton->CreateShip(model);
    lua_pushnumber(L, id);
    return 1;
}

int lua_DeleteShip(lua_State *L)
{
    int32_t id = (int32_t)lua_tonumber(L, 1);
    ServerSingleton->DeleteShip(id);
    return 1;
}