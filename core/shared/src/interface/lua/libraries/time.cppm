// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

#include "pragma/lua/core.hpp"

export module pragma.shared:scripting.lua.libraries.time;

export import :util.enums;

export namespace Lua {
	namespace time {
		DLLNETWORK double cur_time(lua_State *l);
		DLLNETWORK double real_time(lua_State *l);
		DLLNETWORK double delta_time(lua_State *l);
		DLLNETWORK uint64_t time_since_epoch(lua_State *l);
		DLLNETWORK int64_t convert_duration(int64_t duration, ::util::DurationType srcType, ::util::DurationType dstType);
	};
};
