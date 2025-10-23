// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <memory>

export module pragma.shared:scripting.lua.classes.net_packet;

export import luabind;
export import pragma.util;

export {
	namespace pragma::ecs {class BaseEntity;}
	class ALSound;
	namespace Lua {
		namespace NetPacket {
			DLLNETWORK void register_class(luabind::class_<::NetPacket> &classDef);
			DLLNETWORK void WriteEntity(lua_State *l, ::NetPacket &packet, pragma::ecs::BaseEntity *hEnt);
			DLLNETWORK void WriteEntity(lua_State *l, ::NetPacket &packet);
			DLLNETWORK void ReadEntity(lua_State *l, ::NetPacket &packet);
			DLLNETWORK void WriteALSound(lua_State *l, ::NetPacket &packet, std::shared_ptr<ALSound> snd);
			DLLNETWORK void ReadALSound(lua_State *l, ::NetPacket &packet);
			DLLNETWORK void GetTimeSinceTransmission(lua_State *l, ::NetPacket &packet);
		};
	};
};
