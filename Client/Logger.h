#pragma once

#include <string>
#include <memory>

namespace x3 {
    
    enum class LogLevel {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3
    };

    // Abstract logging interface
    class ILogger {
    public:
        virtual ~ILogger() = default;
        virtual void Log(LogLevel level, const std::string& message) = 0;
        virtual void SetLogLevel(LogLevel level) = 0;
        virtual LogLevel GetLogLevel() const = 0;
        
        // Convenience methods
        void Debug(const std::string& message) { Log(LogLevel::Debug, message); }
        void Info(const std::string& message) { Log(LogLevel::Info, message); }
        void Warning(const std::string& message) { Log(LogLevel::Warning, message); }
        void Error(const std::string& message) { Log(LogLevel::Error, message); }
    };

    // Logger factory and global access
    class LoggerManager {
    public:
        static void SetLogger(std::unique_ptr<ILogger> logger);
        static ILogger& GetLogger();
        static void SetLogLevel(LogLevel level);
        
    private:
        static std::unique_ptr<ILogger> s_logger;
        static LogLevel s_globalLogLevel;
    };

    // Convenience macros
    #define LOG_DEBUG(msg) x3::LoggerManager::GetLogger().Debug(msg)
    #define LOG_INFO(msg) x3::LoggerManager::GetLogger().Info(msg)
    #define LOG_WARNING(msg) x3::LoggerManager::GetLogger().Warning(msg)
    #define LOG_ERROR(msg) x3::LoggerManager::GetLogger().Error(msg)
}