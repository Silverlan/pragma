// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"


export module pragma.server.scripting.lua.classes.net_packet;

export import pragma.shared;
export import pragma.util;

export namespace Lua {
	namespace NetPacket {
		namespace Server {
			DLLSERVER void register_class(luabind::class_<::NetPacket> &classDef);
			DLLSERVER void WriteALSound(lua_State *l, ::NetPacket &packet, std::shared_ptr<::ALSound> snd);
			DLLSERVER void WriteUniqueEntity(lua_State *l, ::NetPacket &packet, BaseEntity *hEnt);
			DLLSERVER void WriteUniqueEntity(lua_State *l, ::NetPacket &packet);
		};
	};
};
