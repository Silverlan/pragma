// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LTIME_HPP__
#define __LTIME_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace util {
	enum class DurationType : uint32_t;
};
namespace Lua {
	namespace time {
		DLLNETWORK double cur_time(lua_State *l);
		DLLNETWORK double real_time(lua_State *l);
		DLLNETWORK double delta_time(lua_State *l);
		DLLNETWORK uint64_t time_since_epoch(lua_State *l);
		DLLNETWORK int64_t convert_duration(int64_t duration, ::util::DurationType srcType, ::util::DurationType dstType);
	};
};

#endif
