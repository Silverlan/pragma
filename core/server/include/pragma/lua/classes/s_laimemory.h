#ifndef __S_LAIMEMORY_H__
#define __S_LAIMEMORY_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace AIMemory
	{
		DLLSERVER void register_class(lua_State *l,luabind::module_ &mod);
	};
};

#endif
