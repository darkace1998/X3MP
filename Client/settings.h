#pragma once
#include "pugixml.hpp"
#include <iostream>

namespace x3mp
{
	class Settings
	{
	public:
		std::string username;
		std::string ip;
		unsigned short port = 13337;
		bool localmode = false;
		bool debug = false;

		bool Load();
	};
}
