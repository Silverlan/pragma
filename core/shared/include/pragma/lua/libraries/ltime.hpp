/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LTIME_HPP__
#define __LTIME_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace time
	{
		DLLNETWORK int cur_time(lua_State *l);
		DLLNETWORK int real_time(lua_State *l);
		DLLNETWORK int delta_time(lua_State *l);
		DLLNETWORK int time_since_epoch(lua_State *l);
		DLLNETWORK int convert_duration(lua_State *l);
	};
};

#endif
