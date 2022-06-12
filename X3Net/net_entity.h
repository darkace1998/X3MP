#pragma once
#include <cstdint>

namespace x3
{
	namespace net
	{
		struct Entity
		{
			int32_t EntityID = -1;
			int32_t Model = 0;
			int32_t Owner = 0;
			int32_t NetOwnerID = -1;
			int32_t PosX = 0;
			int32_t PosY = 0;
			int32_t PosZ = 0;
			int32_t RotX = 0;
			int32_t RotY = 0;
			int32_t RotZ = 0;
			int32_t RotW = 0;
			int32_t UpX = 0;
			int32_t UpY = 0;
			int32_t UpZ = 0;
			int32_t UpW = 0;
			int32_t LookAtX = 0;
			int32_t LookAtY = 0;
			int32_t LookAtZ = 0;
		};
	}
}