#pragma once

#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschlieﬂen
// Windows-Headerdateien
#include <windows.h>

#include <vector>


namespace mem
{
	void Patch(BYTE* dst, BYTE* src, unsigned int size);
	void Nop(BYTE* dst, unsigned int size);
	uintptr_t FindDMAAddress(uintptr_t ptr, std::vector<unsigned int> offsets);
	bool Detour32(BYTE* toHook, BYTE* ourFunc, const int len); 
	BYTE* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len);
};

