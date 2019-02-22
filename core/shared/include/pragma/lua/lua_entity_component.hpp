#ifndef __LUA_ENTITY_COMPONENT_HPP__
#define __LUA_ENTITY_COMPONENT_HPP__

namespace pragma
{
	namespace Lua
	{
		template<class TComponentHandle>
			bool check_component(lua_State *l,const TComponentHandle &component)
		{
			if(component.expired())
			{
				lua_pushstring(l,"Attempted to use a NULL component");
				lua_error(l);
				return false;
			}
			return true;
		}
	};
};

#endif
