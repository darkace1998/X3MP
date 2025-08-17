#include "Logger.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace x3 {

    // Static member initialization
    std::unique_ptr<ILogger> LoggerManager::s_logger = nullptr;
    LogLevel LoggerManager::s_globalLogLevel = LogLevel::Info;

    // Default console logger implementation
    class ConsoleLogger : public ILogger {
    private:
        LogLevel m_logLevel;
        
        std::string GetTimestamp() const {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;
            
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return ss.str();
        }
        
        std::string GetLevelString(LogLevel level) const {
            switch (level) {
                case LogLevel::Debug: return "[DBG]";
                case LogLevel::Info: return "[INF]";
                case LogLevel::Warning: return "[WRN]";
                case LogLevel::Error: return "[ERR]";
                default: return "[UNK]";
            }
        }
        
    public:
        ConsoleLogger(LogLevel level = LogLevel::Info) : m_logLevel(level) {}
        
        void Log(LogLevel level, const std::string& message) override {
            if (level >= m_logLevel) {
                std::cout << GetTimestamp() << " " << GetLevelString(level) 
                         << " " << message << std::endl;
            }
        }
        
        void SetLogLevel(LogLevel level) override {
            m_logLevel = level;
        }
        
        LogLevel GetLogLevel() const override {
            return m_logLevel;
        }
    };

    void LoggerManager::SetLogger(std::unique_ptr<ILogger> logger) {
        s_logger = std::move(logger);
    }

    ILogger& LoggerManager::GetLogger() {
        if (!s_logger) {
            s_logger = std::make_unique<ConsoleLogger>(s_globalLogLevel);
        }
        return *s_logger;
    }

    void LoggerManager::SetLogLevel(LogLevel level) {
        s_globalLogLevel = level;
        if (s_logger) {
            s_logger->SetLogLevel(level);
        }
    }
}