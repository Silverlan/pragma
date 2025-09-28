// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/lua/luaapi.h"

import pragma.shared;

void LuaComponentEvent::PushArguments(lua_State *l)
{
	for(auto &o : arguments)
		o.push(l);
}
