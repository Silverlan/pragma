// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :core.logging;

std::string Lua::logging::to_string(lua::State *l, int i)
{
	auto status = -1;
	std::string val;
	if(lua_value_to_string(l, i, &status, &val) == false)
		return "unknown";
	return val;
}
