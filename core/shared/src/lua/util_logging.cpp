/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

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
