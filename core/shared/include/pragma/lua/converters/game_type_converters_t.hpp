/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_GAME_TYPE_CONVERTERS_T_HPP__
#define __LUA_GAME_TYPE_CONVERTERS_T_HPP__

#include "pragma/lua/converters/game_type_converters.hpp"

template<typename T, T (*FUNCTION)(lua_State *)>
template<class U>
T luabind::parameter_emplacement_converter<T, FUNCTION>::to_cpp(lua_State *L, U, int /*index*/)
{
	T p = FUNCTION(L);
	return static_cast<T>(p);
}

template<typename T, T (*FUNCTION)(lua_State *)>
template<class U>
int luabind::parameter_emplacement_converter<T, FUNCTION>::match(lua_State *, U, int /*index*/)
{
	return 0;
}

/////////////

template<typename T, typename TConverter>
template<class U>
T luabind::game_object_converter<T, TConverter>::to_cpp(lua_State *L, U u, int index)
{
	return m_converter.to_cpp(L, u, index);
}

template<typename T, typename TConverter>
void luabind::game_object_converter<T, TConverter>::to_lua(lua_State *L, T x)
{
	if constexpr(std::is_pointer_v<T>) {
		if(!x)
			lua_pushnil(L);
		else
			const_cast<T>(x)->GetLuaObject(L).push(L);
	}
	else
		const_cast<T>(x).GetLuaObject(L).push(L);
}

template<typename T, typename TConverter>
template<class U>
int luabind::game_object_converter<T, TConverter>::match(lua_State *L, U u, int index)
{
	return m_converter.match(L, u, index);
}

#endif
