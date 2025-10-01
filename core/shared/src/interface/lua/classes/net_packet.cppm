// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"

export module pragma.shared:scripting.lua.classes.net_packet;

export import pragma.util;

export {
	class BaseEntity;
	class ALSound;
	namespace Lua {
		namespace NetPacket {
			DLLNETWORK void register_class(luabind::class_<::NetPacket> &classDef);
			DLLNETWORK void WriteEntity(lua_State *l, ::NetPacket &packet, BaseEntity *hEnt);
			DLLNETWORK void WriteEntity(lua_State *l, ::NetPacket &packet);
			DLLNETWORK void ReadEntity(lua_State *l, ::NetPacket &packet);
			DLLNETWORK void WriteALSound(lua_State *l, ::NetPacket &packet, std::shared_ptr<ALSound> snd);
			DLLNETWORK void ReadALSound(lua_State *l, ::NetPacket &packet);
			DLLNETWORK void GetTimeSinceTransmission(lua_State *l, ::NetPacket &packet);
		};
	};
};
