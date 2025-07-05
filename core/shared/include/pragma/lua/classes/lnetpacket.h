// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LNETPACKET_H__
#define __LNETPACKET_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <sharedutils/netpacket.hpp>
#include "pragma/entities/baseentity.h"
#include "pragma/audio/alsound.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldatastream.h"

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

#endif
