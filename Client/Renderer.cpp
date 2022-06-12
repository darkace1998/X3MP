#include "Renderer.h"

Renderer::Renderer() 
{
	HRESULT result = D3DXCreateFontA(pDevice, 14, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &pFont);
	if (result != S_OK)
		std::cout << "[ERROR] D3DXCreateFontA didn't work" << std::endl;
}

Renderer::~Renderer()
{
	pFont->Release();
	delete(pFont);
}

void Renderer::DrawFilledRect(int x, int y, int w, int h, D3DCOLOR col) const
{
	D3DRECT rect = { x,y,x + w,y + w };
	pDevice->Clear(1, &rect, D3DCLEAR_TARGET, col, 0, 0);
}

void Renderer::DrawTextString(int x, int y, const std::string str) const
{
	DrawTextString(x, y, str, 255, 255, 255, 255);
}

void Renderer::DrawTextString(int x, int y, const std::string str, byte A, byte R, byte G, byte B) const
{
	if (pFont == nullptr)
		return;
	RECT TextRect = {x,y,0,0};
	int drawResult = pFont->DrawTextA(0, str.c_str(), str.length(), &TextRect, DT_NOCLIP, D3DCOLOR_ARGB(A, R, G, B));
	if (drawResult == 0)
		std::cout << "[ERROR] DrawTextA didn't work" << std::endl;
}