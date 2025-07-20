#include "Config.h"
#include <sstream>
#include <algorithm>

Config::Config(const std::string& filename) : configFile(filename) {
}

bool Config::load() {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file '" << configFile 
                  << "'. Using default values." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '{' || line[0] == '}' || line[0] == '#') {
            continue;
        }
        parseConfigLine(line);
    }

    file.close();
    return true;
}

bool Config::save() const {
    std::ofstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create config file '" << configFile << "'" << std::endl;
        return false;
    }

    file << "{\n";
    file << "  \"server_port\": " << server_port << ",\n";
    file << "  \"max_players\": " << max_players << ",\n";
    file << "  \"server_name\": \"" << server_name << "\",\n";
    file << "  \"log_level\": \"" << log_level << "\",\n";
    file << "  \"tick_rate\": " << tick_rate << ",\n";
    file << "  \"timeout_seconds\": " << timeout_seconds << ",\n";
    file << "  \"enable_debug\": " << (enable_debug ? "true" : "false") << ",\n";
    file << "  \"lua_script\": \"" << lua_script << "\",\n";
    file << "  \"bind_address\": \"" << bind_address << "\"\n";
    file << "}\n";

    file.close();
    return true;
}

void Config::parseConfigLine(const std::string& line) {
    try {
        if (line.find("\"server_port\"") != std::string::npos) {
            server_port = extractIntValue(line);
        } else if (line.find("\"max_players\"") != std::string::npos) {
            max_players = extractIntValue(line);
        } else if (line.find("\"server_name\"") != std::string::npos) {
            server_name = extractStringValue(line);
        } else if (line.find("\"log_level\"") != std::string::npos) {
            log_level = extractStringValue(line);
        } else if (line.find("\"tick_rate\"") != std::string::npos) {
            tick_rate = extractIntValue(line);
        } else if (line.find("\"timeout_seconds\"") != std::string::npos) {
            timeout_seconds = extractIntValue(line);
        } else if (line.find("\"enable_debug\"") != std::string::npos) {
            enable_debug = extractBoolValue(line);
        } else if (line.find("\"lua_script\"") != std::string::npos) {
            lua_script = extractStringValue(line);
        } else if (line.find("\"bind_address\"") != std::string::npos) {
            bind_address = extractStringValue(line);
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error parsing config line '" << line << "': " << e.what() << std::endl;
    }
}

std::string Config::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string Config::extractStringValue(const std::string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        throw std::runtime_error("No colon found in config line");
    }

    std::string value = line.substr(colonPos + 1);
    value = trim(value);
    
    // Remove quotes and comma
    if (value.back() == ',') {
        value.pop_back();
    }
    
    if (value.size() >= 2 && value.front() == '\"' && value.back() == '\"') {
        return value.substr(1, value.length() - 2);
    }
    
    throw std::runtime_error("String value not properly quoted");
}

int Config::extractIntValue(const std::string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        throw std::runtime_error("No colon found in config line");
    }

    std::string value = line.substr(colonPos + 1);
    value = trim(value);
    
    // Remove comma if present
    if (value.back() == ',') {
        value.pop_back();
        value = trim(value);
    }
    
    return std::stoi(value);
}

bool Config::extractBoolValue(const std::string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        throw std::runtime_error("No colon found in config line");
    }

    std::string value = line.substr(colonPos + 1);
    value = trim(value);
    
    // Remove comma if present
    if (value.back() == ',') {
        value.pop_back();
        value = trim(value);
    }
    
    return value == "true";
}

void Config::print() const {
    std::cout << "=== Server Configuration ===" << std::endl;
    std::cout << "Server Port: " << server_port << std::endl;
    std::cout << "Max Players: " << max_players << std::endl;
    std::cout << "Server Name: " << server_name << std::endl;
    std::cout << "Log Level: " << log_level << std::endl;
    std::cout << "Tick Rate: " << tick_rate << std::endl;
    std::cout << "Timeout (seconds): " << timeout_seconds << std::endl;
    std::cout << "Debug Enabled: " << (enable_debug ? "Yes" : "No") << std::endl;
    std::cout << "Lua Script: " << lua_script << std::endl;
    std::cout << "Bind Address: " << bind_address << std::endl;
    std::cout << "=============================" << std::endl;
}