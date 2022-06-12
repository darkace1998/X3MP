#include "Chatbox.h"

void Chatbox::SendChatMessage(std::string message, byte A, byte R, byte G, byte B)
{
	lines.push_back( std::make_unique<Message>(message, A, R, G, B));
}

void Chatbox::SendChatMessage(const char* message, byte A, byte R, byte G, byte B)
{
	lines.push_back(std::make_unique<Message>(std::string(message), A, R, G, B));
}

void Chatbox::Draw(Renderer& renderer) const
{
	for (unsigned int i = 0; i < 10; i++)
	{
		if (i >= lines.size())
			break;
		renderer.DrawTextString(100, 100 + i * 16, lines[i]->message, lines[i]->A, lines[i]->R, lines[i]->G, lines[i]->B);
	}
}
