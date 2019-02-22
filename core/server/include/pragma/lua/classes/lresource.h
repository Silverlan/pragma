#ifndef __LRESOURCE_H__
#define __LRESOURCE_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace resource
	{
		DLLSERVER int add_file(lua_State *l);
		DLLSERVER int add_lua_file(lua_State *l);
		DLLSERVER int get_list(lua_State *l);
	};
};

#endif
