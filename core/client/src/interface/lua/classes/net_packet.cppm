// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/classes/ldef_netpacket.h"

export module pragma.client.scripting.lua.classes.net_packet;

export namespace Lua {
	namespace NetPacket {
		namespace Client {
			DLLCLIENT void register_class(luabind::class_<::NetPacket> &classDef);
			DLLCLIENT void ReadUniqueEntity(lua_State *l, ::NetPacket &packet, luabind::object o);
		};
	};
};
