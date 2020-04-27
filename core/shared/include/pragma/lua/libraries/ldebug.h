/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LDEBUG_H__
#define __LDEBUG_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace debug
	{
		DLLNETWORK int stackdump(lua_State *l);
		DLLNETWORK int collectgarbage(lua_State *l);
		DLLNETWORK int move_state_to_string(lua_State *l);
		DLLNETWORK int enable_remote_debugging(lua_State *l);
	};
};

#endif
