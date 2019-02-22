#ifndef __C_LLOCALE_H__
#define __C_LLOCALE_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace Locale
	{
		DLLCLIENT int get_text(lua_State *l);
		DLLCLIENT int load(lua_State *l);
		DLLCLIENT int get_language(lua_State *l);
	};
};

#endif