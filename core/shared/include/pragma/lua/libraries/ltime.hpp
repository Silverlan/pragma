#ifndef __LTIME_HPP__
#define __LTIME_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace time
	{
		DLLNETWORK int cur_time(lua_State *l);
		DLLNETWORK int real_time(lua_State *l);
		DLLNETWORK int delta_time(lua_State *l);
		DLLNETWORK int time_since_epoch(lua_State *l);
		DLLNETWORK int convert_duration(lua_State *l);
	};
};

#endif
