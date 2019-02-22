#ifndef __LUA_HANDLE_HPP__
#define __LUA_HANDLE_HPP__

struct lua_State;
namespace Lua
{
	template<class T>
		bool CheckWHandle(lua_State *l,const util::WeakHandle<T> &handle)
	{
		if(handle.expired())
		{
			Lua::PushString(l,"Attempted to use a NULL handle");
			lua_error(l);
			return false;
		}
		return true;
	}
};

#endif
