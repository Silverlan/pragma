#include "stdafx_shared.h"
#include "pragma/lua/l_entity_handles.hpp"

bool Lua::CheckComponentHandle(lua_State *l,const BaseEntityComponentHandle &handle)
{
	if(handle.expired())
	{
		Lua::PushString(l,"Attempted to use a NULL component handle");
		lua_error(l);
		return false;
	}
	return true;
}
