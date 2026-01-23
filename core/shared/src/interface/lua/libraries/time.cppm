// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.time;

export import :util.enums;
export import pragma.lua;

export namespace Lua {
	namespace time {
		DLLNETWORK double cur_time(lua::State *l);
		DLLNETWORK double real_time(lua::State *l);
		DLLNETWORK double delta_time(lua::State *l);
		DLLNETWORK uint64_t time_since_epoch(lua::State *l);
		DLLNETWORK int64_t convert_duration(int64_t duration, pragma::util::DurationType srcType, pragma::util::DurationType dstType);
	};
};
