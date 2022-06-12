#pragma once
#include <windows.h>
#include "X3Classes.h"
#include "astruct_9.h"
#include "mem.h"

namespace x3
{
	struct struct_1
	{
		BYTE byte;
		int _class;
		BYTE byte2;
		int type;
		int something;
		BYTE idk;
	};

	//Gamefunctions
	typedef x3::Entity* (__cdecl* _AllocateEntitySpace)(size_t size);
	typedef x3::Entity* (__stdcall* _CreateInSectorEntity)(x3::Entity* entity, int type);
	typedef x3::Entity* (__thiscall* _SetEntityInSector)(x3::Entity* entity, x3::Sector* targetSector);
	typedef int(__cdecl* _SetSimulatorParam)(int param_1, int param_2, int param_3, const char* param_4, short param_5, int param_6);
	typedef const char* (__fastcall* _GetSomeText)(int param_1, int param_2);
	typedef void (__stdcall* _DeleteEntity)(x3::Entity* entity);
	typedef void* (__thiscall* _LoadThisObject)(void* object, const char* path);
	typedef int (__stdcall* _SetSectorBackground)(void* object);

	extern _AllocateEntitySpace AllocateEntitySpace;
	extern _CreateInSectorEntity CreateInSectorEntity;
	extern _SetEntityInSector SetEntityInSector;
	extern _SetSimulatorParam SetSimulatorParam;
	extern _GetSomeText GetSomeText;
	extern _DeleteEntity DeleteEntity;
	extern _LoadThisObject LoadThisObject;
	extern _SetSectorBackground SetSectorBackground;

	//Hooks
	typedef int(__cdecl* tSomeUpdate)(int param_1, int param_2, int updateType, int param_4, struct_1* updateData);
	typedef int(__stdcall* tDeleteEntityLoop)(void* param_1);

	extern tSomeUpdate SomeUpdate;
	extern tDeleteEntityLoop DeleteEntityLoop;

	void InitFunctionPointer(uintptr_t moduleBase);
	void HookFunctions(BYTE* hook_SomeUpdate, BYTE* hook_DeleteEntityLoop);
}