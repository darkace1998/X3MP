#pragma once
#include <string>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9core.h>
#include "directx.h"
#include <iostream>

class Renderer
{
	ID3DXFont* pFont{ nullptr };

public:
	Renderer();
	void DrawFilledRect(int x, int y, int w, int h, D3DCOLOR col) const;
	void DrawTextString(int x, int y, const std::string str) const;
	void DrawTextString(int x, int y, const std::string str, byte A, byte R, byte G, byte B) const;
	~Renderer();
};