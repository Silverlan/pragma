/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LNETPACKET_H__
#define __LNETPACKET_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <sharedutils/netpacket.hpp>
#include "pragma/entities/baseentity.h"
#include "pragma/audio/alsound.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldatastream.h"

namespace Lua
{
	namespace NetPacket
	{
		DLLNETWORK void register_class(luabind::class_<::NetPacket> &classDef);
		DLLNETWORK void WriteEntity(lua_State *l,::NetPacket &packet,BaseEntity *hEnt);
		DLLNETWORK void WriteEntity(lua_State *l,::NetPacket &packet);
		DLLNETWORK void ReadEntity(lua_State *l,::NetPacket &packet);
		DLLNETWORK void WriteALSound(lua_State *l,::NetPacket &packet,std::shared_ptr<ALSound> snd);
		DLLNETWORK void ReadALSound(lua_State *l,::NetPacket &packet);
		DLLNETWORK void GetTimeSinceTransmission(lua_State *l,::NetPacket &packet);
	};
};

#endif