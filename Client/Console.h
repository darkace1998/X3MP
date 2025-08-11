#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <array>
#include <Windows.h>

namespace x3
{
	enum class MessageLevel
	{
		Debug,
		Info,
		Error
	};

	class Console
	{
	private:
		FILE* f;
		Console();
		static Console* instance;

	public:
		// Delete copy constructor and assignment operator
		Console(const Console&) = delete;
		Console& operator=(const Console&) = delete;

		static Console& GetInstance();
		void Log(std::string, MessageLevel lvl);
		void Log(const char* str, MessageLevel lvl);
		void Close();
	};
}