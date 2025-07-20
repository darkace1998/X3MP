#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

// Simple JSON-like config parser for basic key-value pairs
class Config {
private:
    std::string configFile;
    
    // Default configuration values
    int server_port = 13337;
    int max_players = 32;
    std::string server_name = "X3MP Server";
    std::string log_level = "info";
    int tick_rate = 60;
    int timeout_seconds = 30;
    bool enable_debug = false;
    std::string lua_script = "luascript.lua";
    std::string bind_address = "0.0.0.0";

    void parseConfigLine(const std::string& line);
    std::string trim(const std::string& str);
    std::string extractStringValue(const std::string& line);
    int extractIntValue(const std::string& line);
    bool extractBoolValue(const std::string& line);

public:
    explicit Config(const std::string& filename = "config.json");
    
    // Getters
    int getServerPort() const { return server_port; }
    int getMaxPlayers() const { return max_players; }
    const std::string& getServerName() const { return server_name; }
    const std::string& getLogLevel() const { return log_level; }
    int getTickRate() const { return tick_rate; }
    int getTimeoutSeconds() const { return timeout_seconds; }
    bool isDebugEnabled() const { return enable_debug; }
    const std::string& getLuaScript() const { return lua_script; }
    const std::string& getBindAddress() const { return bind_address; }
    
    // Load configuration from file
    bool load();
    
    // Save current configuration to file
    bool save() const;
    
    // Print current configuration
    void print() const;
};