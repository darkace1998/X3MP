#pragma once
#include "directx.h"
#include <memory>
#include <vector>
#include <string>
#include "Renderer.h"

class Message
{
public:
	byte A;
	byte R;
	byte G;
	byte B;
	std::string message;

	Message(std::string message, byte A, byte R, byte G, byte B)
	{
		this->message = message;
		this->A = A;
		this->R = R;
		this->G = G;
		this->B = B;
	}
};

class Chatbox
{
	std::vector<std::unique_ptr<Message>> lines{};

public:
	void SendChatMessage(std::string message, byte A = 255, byte R = 255, byte G = 255, byte B = 255);
	void SendChatMessage(const char* message, byte A = 255, byte R = 255, byte G = 255, byte B = 255);

	void Draw(Renderer& renderer) const;
};

