#ifndef __C_LGLOBAL_H__
#define __C_LGLOBAL_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
DLLCLIENT int Lua_ServerTime(lua_State *l);
DLLCLIENT int Lua_FrameTime(lua_State *l);

#endif