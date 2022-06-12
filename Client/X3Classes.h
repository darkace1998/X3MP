#pragma once
#include <cstdint>

// Created with ReClass.NET 1.2 by KN4CK3R

namespace x3 {
	// Created with ReClass.NET 1.2 by KN4CK3R

	class Base
	{
	public:
		class EntityManager* EntityManager; //0x0000
	}; //Size: 0x0004

	class EntityManager
	{
	public:
		char pad_0000[16]; //0x0000
		class Entity* EntityList; //0x0010
	}; //Size: 0x0014

	class Entity
	{
	public:
		class Entity* NextLink; //0x0000
		class Entity* PrevLink; //0x0004
		uint32_t ListID; //0x0008
		char* ShipType; //0x000C
		char pad_0010[8]; //0x0010
		int32_t RO_RotX; //0x0018
		int32_t RO_RotY; //0x001C
		int32_t RO_RotZ; //0x0020
		int32_t RudderYaw; //0x0024
		int32_t RudderPitch; //0x0028
		int32_t RudderRoll; //0x002C
		char pad_0030[12]; //0x0030
		uint32_t Owner; //0x003C
		uint32_t Pilot; //0x0040
		char pad_0044[4]; //0x0044
		uint16_t N00000780; //0x0048
		uint16_t ShipTypeID; //0x004A
		char pad_004C[4]; //0x004C
		void* N00000782; //0x0050
		class Sector* Sector; //0x0054
		char pad_0058[8]; //0x0058
		int32_t RudderX; //0x0060
		int32_t RudderY; //0x0064
		int32_t RudderZ; //0x0068
		uint32_t N00000789; //0x006C 0x1 Normal | 0x18 MouseControl
		class WorldData* WorldData; //0x0070
		char pad_0074[116]; //0x0074
		uint32_t RO_Speed; //0x00E8
		uint32_t Hull; //0x00EC
		char pad_00F0[72]; //0x00F0
	}; //Size: 0x0138

	class WorldData
	{
	public:
		char pad_0000[28]; //0x0000
		void* N000007F7; //0x001C
		char pad_0020[16]; //0x0020
		int32_t PosX; //0x0030
		int32_t PosY; //0x0034
		int32_t PosZ; //0x0038
		char pad_003C[4]; //0x003C
		int32_t UpQuaternionX; //0x0040
		int32_t UpQuaternionY; //0x0044
		int32_t UpQuaternionZ; //0x0048
		int32_t UpQuaternionW; //0x004C
		int32_t RotQuaternionX; //0x0050
		int32_t RotQuaternionY; //0x0054
		int32_t RotQuaternionZ; //0x0058
		int32_t RotQuaternionW; //0x005C
		int32_t LookAtX; //0x0060
		int32_t LookAtY; //0x0064
		int32_t LookAtZ; //0x0068
		char pad_006C[148]; //0x006C
	}; //Size: 0x0100

	class ShipType
	{
	public:
		char Name[1]; //0x0000
		char pad_0001[252]; //0x0001
	}; //Size: 0x00FD

	class Base2
	{
	public:
		class Sim *N000010EA; //0x0000
		char pad_0004[8604]; //0x0004
	}; //Size: 0x21A0

	class Sim
	{
	public:
		char pad_0000[16]; //0x0000
		class Sector* Sector; //0x0010
		class N00000DAE* N00000B10; //0x0014
		class N00000C15* N00000B11; //0x0018
		char pad_001C[52]; //0x001C
		int32_t TraveledMeterCount; //0x0050
		int32_t Traveled; //0x0054
		char pad_0058[16]; //0x0058
		int32_t N00000B24; //0x0068
		char pad_006C[440]; //0x006C
	}; //Size: 0x7C04

	class Sector
	{
	public:
		char pad_0000[12]; //0x0000
		char* N00000B87; //0x000C
		char pad_0010[156]; //0x0010
		class N00001481* N00000BAF; //0x00AC
	}; //Size: 0x00B0

	class N00000C15
	{
	public:
		char pad_0000[260]; //0x0000
	}; //Size: 0x0104

	class N00000DAE
	{
	public:
		char pad_0000[132]; //0x0000
	}; //Size: 0x0084

	class N00000E27
	{
	public:
		char pad_0000[4]; //0x0000
	}; //Size: 0x0004

	class N00001977
	{
	public:
		char pad_0000[4]; //0x0000
	}; //Size: 0x0004

	class N00001481
	{
	public:
		char pad_0000[132]; //0x0000
		class N000014E7* N000014A4; //0x0084
	}; //Size: 0x0088

	class N000014E7
	{
	public:
		char pad_0000[16]; //0x0000
		class N0000151D* N000014ED; //0x0010
	}; //Size: 0x0014

	class N0000151D
	{
	public:
		char pad_0000[204]; //0x0000
		class N00001583* N00001552; //0x00CC
	}; //Size: 0x00D0

	class N00001583
	{
	public:
		char pad_0000[480]; //0x0000
		Entity* Target; //0x01E0
	}; //Size: 0x01E4
}