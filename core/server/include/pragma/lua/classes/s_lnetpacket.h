// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_LNETPACKET_H__
#define __S_LNETPACKET_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/classes/ldef_netpacket.h"

namespace Lua {
	namespace NetPacket {
		namespace Server {
			DLLSERVER void register_class(luabind::class_<::NetPacket> &classDef);
			DLLSERVER void WriteALSound(lua_State *l, ::NetPacket &packet, std::shared_ptr<ALSound> snd);
			DLLSERVER void WriteUniqueEntity(lua_State *l, ::NetPacket &packet, BaseEntity *hEnt);
			DLLSERVER void WriteUniqueEntity(lua_State *l, ::NetPacket &packet);
		};
	};
};

#endif
