#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/libraries/los.h"
#include <pragma/lua/luaapi.h>

int Lua::os::time_since_epoch(lua_State *l)
{
	auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	Lua::PushInt(l,ms);
	return 1;
}
