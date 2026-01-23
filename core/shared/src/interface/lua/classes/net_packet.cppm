// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.net_packet;

export import pragma.lua;

export {
	namespace pragma::ecs {
		class BaseEntity;
	}
	namespace pragma::audio {
		class ALSound;
	}
	namespace Lua {
		namespace NetPacket {
			DLLNETWORK void register_class(luabind::class_<::NetPacket> &classDef);
			DLLNETWORK void WriteEntity(lua::State *l, ::NetPacket &packet, pragma::ecs::BaseEntity *hEnt);
			DLLNETWORK void WriteEntity(lua::State *l, ::NetPacket &packet);
			DLLNETWORK void ReadEntity(lua::State *l, ::NetPacket &packet);
			DLLNETWORK void WriteALSound(lua::State *l, ::NetPacket &packet, std::shared_ptr<pragma::audio::ALSound> snd);
			DLLNETWORK void ReadALSound(lua::State *l, ::NetPacket &packet);
			DLLNETWORK void GetTimeSinceTransmission(lua::State *l, ::NetPacket &packet);
		};
	};
};
