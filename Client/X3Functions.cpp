#include "X3Functions.h"

namespace x3 
{
	_AllocateEntitySpace AllocateEntitySpace;
	_CreateInSectorEntity CreateInSectorEntity;
	_SetEntityInSector SetEntityInSector;
	_SetSimulatorParam SetSimulatorParam;
	_GetSomeText GetSomeText;
	_DeleteEntity DeleteEntity;
	_LoadThisObject LoadThisObject;
	_SetSectorBackground SetSectorBackground;

	tSomeUpdate SomeUpdate;
	tDeleteEntityLoop DeleteEntityLoop;

	void InitFunctionPointer(uintptr_t moduleBase)
	{
		//Adding function pointer
		AllocateEntitySpace = (_AllocateEntitySpace)(moduleBase + 0xe6a40);
		CreateInSectorEntity = (_CreateInSectorEntity)(moduleBase + 0x3f900);
		SetEntityInSector = (_SetEntityInSector)(moduleBase + 0x49510);
		SetSimulatorParam = (_SetSimulatorParam)(moduleBase + 0x9f4c0);
		GetSomeText = (_GetSomeText)(moduleBase + 0xab200);
		DeleteEntity = (_DeleteEntity)(moduleBase + 0x3fca0);
		LoadThisObject = (_LoadThisObject)(moduleBase + 0x07c90);
		SetSectorBackground = (_SetSectorBackground)(moduleBase + 0x6e400);
	}

	void HookFunctions(BYTE* hook_SomeUpdate, BYTE* hook_DeleteEntityLoop)
	{
		x3::SomeUpdate = (x3::tSomeUpdate)0x00460630;
		SomeUpdate = (x3::tSomeUpdate)mem::TrampHook32((BYTE*)SomeUpdate, hook_SomeUpdate, 6);
		x3::DeleteEntityLoop = (x3::tDeleteEntityLoop)0x0045b660;
		DeleteEntityLoop = (x3::tDeleteEntityLoop)mem::TrampHook32((BYTE*)DeleteEntityLoop, hook_DeleteEntityLoop, 6);
		//x3::SetSimulatorParam = (x3::tSetSimulatorParam)0x0049f4c0;
		//SetSimulatorParam = (x3::tSetSimulatorParam)mem::TrampHook32((BYTE*)SetSimulatorParam, (BYTE*)hook_SetSimulatorParam, 7);
	}
}