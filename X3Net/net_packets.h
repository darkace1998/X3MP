#pragma once
#include "net_message.h"

namespace x3 {
	namespace net {
		enum class PacketType {
			Connect,
			CreateShip,
			DeleteShip,
			CreateStar,
			ShipUpdate,
			ConnectAcknowledge,
			ChatMessage,
			PlayerChatEnter
		};

		struct Packet {
			virtual ~Packet() = default;
			PacketType type{};
			size_t size{};
		};

		struct ShipUpdate : Packet {
			int32_t ShipID = 0;
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

		struct Connect : Packet {
			int16_t Model = 0;
			char Name[64];
		};

		struct ConnectAcknowledge : Packet {
			int32_t ClientID = -1;
			int32_t ShipID = -1;
		};

		struct CreateShip : Packet {
			int32_t ShipID = 0;
			int32_t Model = 0;
			int32_t Owner = 0;
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

		struct DeleteShip : Packet {
			int32_t ShipID = 0;
		};

		struct CreateStar : Packet {
			int32_t StarID = 0;
			int32_t Model = 0;
			int32_t PosX = 0;
			int32_t PosY = 0;
			int32_t PosZ = 0;
		};

		struct ChatMessage : Packet {
			uint8_t A = 255;
			uint8_t R = 255;
			uint8_t G = 255;
			uint8_t B = 255;
			char Message[512];
		};
		struct PlayerChatEnter: Packet {
			char Message[512];
		};
	}
}