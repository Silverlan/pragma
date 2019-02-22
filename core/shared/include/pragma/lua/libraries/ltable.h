#ifndef __LTABLE_H__
#define __LTABLE_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace table
	{
		DLLNETWORK int has_value(lua_State *l);
		DLLNETWORK int random(lua_State *l);
	};
};

#endif