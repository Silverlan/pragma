#ifndef __LNET_HPP__
#define __LNET_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace net
	{
		DLLNETWORK void RegisterLibraryEnums(lua_State *l);
		DLLNETWORK int32_t register_event(lua_State *l);
	};
};

#endif
