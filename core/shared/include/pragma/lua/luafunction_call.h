#ifndef __LUAFUNCTION_CALL_H__
#define __LUAFUNCTION_CALL_H__

#include "pragma/lua/luafunction.h"
#include <luasystem.h>

template<class T,typename... TARGS>
	T LuaFunction::Call(TARGS ...args)
{
	auto &r = m_luaFunction;
#ifndef LUABIND_NO_EXCEPTIONS
	try
	{
#endif
		return static_cast<T>(luabind::call_function<T>(*r,std::forward<TARGS>(args)...));
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(const luabind::error&)
	{
		Lua::HandleLuaError(r->interpreter());
	}
	catch(const luabind::cast_failed&)
	{
		return T();
	}
#endif
	return T();
}
template<class T,typename... TARGS>
	bool LuaFunction::Call(T *ret,TARGS ...args)
{
	auto &r = m_luaFunction;
#ifndef LUABIND_NO_EXCEPTIONS
	try
	{
#endif
		*ret = static_cast<T>(luabind::call_function<T>(*r,std::forward<TARGS>(args)...));
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error&)
	{
		Lua::HandleLuaError(r->interpreter());
		return false;
	}
	catch(std::exception&)
	{
		return false;
	}
#endif
	auto *state = r->interpreter();
	r->push(state);
	auto cret = (lua_iscfunction(state,-1) == 0) ? true : false;
	Lua::Pop(state,1);
	return cret;
}

#endif