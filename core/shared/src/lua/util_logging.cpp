// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/lua/util_logging.hpp"

std::string Lua::logging::to_string(lua_State *l, int i)
{
	auto status = -1;
	std::string val;
	if(Lua::lua_value_to_string(l, i, &status, &val) == false)
		return "unknown";
	return val;
}
