/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LNETPACKET_H__
#define __C_LNETPACKET_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/classes/ldef_netpacket.h"

namespace Lua
{
	namespace NetPacket
	{
		namespace Client
		{
			DLLCLIENT void register_class(luabind::class_<::NetPacket> &classDef);
			DLLCLIENT void ReadUniqueEntity(lua_State *l,::NetPacket &packet,luabind::object o);
		};
	};
};

#endif