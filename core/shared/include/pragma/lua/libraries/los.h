#ifndef __LOS_H__
#define __LOS_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace os
	{
		DLLNETWORK int time_since_epoch(lua_State *l);
	};
};

#endif
