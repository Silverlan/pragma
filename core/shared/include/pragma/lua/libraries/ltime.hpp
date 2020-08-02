/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LTIME_HPP__
#define __LTIME_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace util {enum class DurationType : uint32_t;};
namespace Lua
{
	namespace time
	{
		DLLNETWORK double cur_time(lua_State *l);
		DLLNETWORK double real_time(lua_State *l);
		DLLNETWORK double delta_time(lua_State *l);
		DLLNETWORK uint64_t time_since_epoch(lua_State *l);
		DLLNETWORK int64_t convert_duration(int64_t duration,::util::DurationType srcType,::util::DurationType dstType);
	};
};

#endif
