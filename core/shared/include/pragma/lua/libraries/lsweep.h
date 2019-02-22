#ifndef __LSWEEP_H__
#define __LSWEEP_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
DLLNETWORK int Lua_sweep_AABBWithAABB(lua_State *l);
DLLNETWORK int Lua_sweep_AABBWithPlane(lua_State *l);
#endif