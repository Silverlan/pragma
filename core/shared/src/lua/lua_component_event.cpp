#include "stdafx_shared.h"
#include "pragma/lua/lua_component_event.hpp"

void LuaComponentEvent::PushArguments(lua_State *l)
{
	for(auto &o : arguments)
		o.push(l);
}
