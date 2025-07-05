// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/lua/lua_component_event.hpp"

void LuaComponentEvent::PushArguments(lua_State *l)
{
	for(auto &o : arguments)
		o.push(l);
}
