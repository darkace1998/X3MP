#include <gtest/gtest.h>
#include "Logger.h"
#include <sstream>

class TestLogger : public x3::ILogger {
private:
    std::stringstream m_output;
    x3::LogLevel m_level;

public:
    TestLogger(x3::LogLevel level = x3::LogLevel::Debug) : m_level(level) {}

    void Log(x3::LogLevel level, const std::string& message) override {
        if (level >= m_level) {
            m_output << static_cast<int>(level) << ":" << message << "\n";
        }
    }

    void SetLogLevel(x3::LogLevel level) override {
        m_level = level;
    }

    x3::LogLevel GetLogLevel() const override {
        return m_level;
    }

    std::string GetOutput() const {
        return m_output.str();
    }

    void Clear() {
        m_output.str("");
        m_output.clear();
    }
};

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset logger state for each test
        testLogger = std::make_unique<TestLogger>();
        x3::LoggerManager::SetLogger(std::make_unique<TestLogger>());
    }

    void TearDown() override {
        testLogger.reset();
    }

    std::unique_ptr<TestLogger> testLogger;
};

TEST_F(LoggerTest, BasicLogging) {
    auto testLoggerPtr = std::make_unique<TestLogger>();
    auto* rawPtr = testLoggerPtr.get();
    
    x3::LoggerManager::SetLogger(std::move(testLoggerPtr));
    
    x3::LoggerManager::GetLogger().Debug("Debug message");
    x3::LoggerManager::GetLogger().Info("Info message");
    x3::LoggerManager::GetLogger().Warning("Warning message");
    x3::LoggerManager::GetLogger().Error("Error message");
    
    std::string output = rawPtr->GetOutput();
    EXPECT_TRUE(output.find("Debug message") != std::string::npos);
    EXPECT_TRUE(output.find("Info message") != std::string::npos);
    EXPECT_TRUE(output.find("Warning message") != std::string::npos);
    EXPECT_TRUE(output.find("Error message") != std::string::npos);
}

TEST_F(LoggerTest, LogLevelFiltering) {
    auto testLoggerPtr = std::make_unique<TestLogger>(x3::LogLevel::Warning);
    auto* rawPtr = testLoggerPtr.get();
    
    x3::LoggerManager::SetLogger(std::move(testLoggerPtr));
    
    x3::LoggerManager::GetLogger().Debug("Debug message");
    x3::LoggerManager::GetLogger().Info("Info message");
    x3::LoggerManager::GetLogger().Warning("Warning message");
    x3::LoggerManager::GetLogger().Error("Error message");
    
    std::string output = rawPtr->GetOutput();
    EXPECT_TRUE(output.find("Debug message") == std::string::npos);
    EXPECT_TRUE(output.find("Info message") == std::string::npos);
    EXPECT_TRUE(output.find("Warning message") != std::string::npos);
    EXPECT_TRUE(output.find("Error message") != std::string::npos);
}

TEST_F(LoggerTest, LogLevelChanging) {
    auto testLoggerPtr = std::make_unique<TestLogger>(x3::LogLevel::Info);
    auto* rawPtr = testLoggerPtr.get();
    
    x3::LoggerManager::SetLogger(std::move(testLoggerPtr));
    
    // Initially at Info level
    x3::LoggerManager::GetLogger().Debug("Debug 1");
    x3::LoggerManager::GetLogger().Info("Info 1");
    
    // Change to Debug level
    x3::LoggerManager::SetLogLevel(x3::LogLevel::Debug);
    x3::LoggerManager::GetLogger().Debug("Debug 2");
    x3::LoggerManager::GetLogger().Info("Info 2");
    
    std::string output = rawPtr->GetOutput();
    EXPECT_TRUE(output.find("Debug 1") == std::string::npos);
    EXPECT_TRUE(output.find("Info 1") != std::string::npos);
    EXPECT_TRUE(output.find("Debug 2") != std::string::npos);
    EXPECT_TRUE(output.find("Info 2") != std::string::npos);
}