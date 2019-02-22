#ifndef __LDEBUG_H__
#define __LDEBUG_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace debug
	{
		DLLNETWORK int stackdump(lua_State *l);
		DLLNETWORK int collectgarbage(lua_State *l);
		DLLNETWORK int move_state_to_string(lua_State *l);
		DLLNETWORK int enable_remote_debugging(lua_State *l);
	};
};

#endif
