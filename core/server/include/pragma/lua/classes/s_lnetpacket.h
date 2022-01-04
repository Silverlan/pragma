/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LNETPACKET_H__
#define __S_LNETPACKET_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/classes/ldef_netpacket.h"

namespace Lua
{
	namespace NetPacket
	{
		namespace Server
		{
			DLLSERVER void register_class(luabind::class_<::NetPacket> &classDef);
			DLLSERVER void WriteALSound(lua_State *l,::NetPacket &packet,std::shared_ptr<ALSound> snd);
			DLLSERVER void WriteUniqueEntity(lua_State *l,::NetPacket &packet,BaseEntity *hEnt);
			DLLSERVER void WriteUniqueEntity(lua_State *l,::NetPacket &packet);
		};
	};
};

#endif