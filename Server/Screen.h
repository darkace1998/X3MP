#pragma once

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#endif
#include <cstddef>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include <queue>

class Screen
{
private:
    static bool Running;
    static std::queue<std::string> cmdBuffer;
    static bool LogToFile;
public:
    static std::thread inputThread;
    static std::unique_ptr<WINDOW> logwindow;
    static std::unique_ptr<WINDOW> inputwindow;

public:
    static void Start(bool logToFile = true);
    static void Stop();
    static void Log(const std::string& message, bool newline = true);
    static void Log(const char* message, bool newline = true);
    static void LogError(const std::string& message, bool newline = true);
    static void LogError(const char* message, bool newline = true);
    static void LogDebug(const std::string& message, bool newline = true);
    static void LogDebug(const char* message, bool newline = true);
    static std::string PollCommand();

private:
    static void GetInput();
};