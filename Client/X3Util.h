#pragma once
#include "X3Classes.h"
#include <algorithm>
#include "defines.h"
#include <vector>
#include <array>

namespace x3
{
	namespace util
	{
		void hook_DeleteEntityLoop(void* param_1);

		void DeleteEntity(x3::Entity* entity);
		const bool CheckShipPointer(x3::Entity* ptr, std::array<Entity*, MAX_ENTITIES>& entities);
	}
}