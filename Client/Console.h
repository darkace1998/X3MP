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
	public:
		Console();
		void Log(std::string, MessageLevel lvl);
		void Log(const char* str, MessageLevel lvl);
		void Close();
	};
}