// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include <luabind/object.hpp>

#ifndef __LUA_GLOBAL_STRING_CONVERTER_T_HPP__
#define __LUA_GLOBAL_STRING_CONVERTER_T_HPP__

template<typename U>
pragma::GString luabind::default_converter<pragma::GString>::to_cpp(lua_State *L, U u, int index)
{
	return {luaL_checkstring(L, index)};
}

template<class U>
int luabind::default_converter<pragma::GString>::match(lua_State *l, U u, int index)
{
	return lua_isstring(l, index) ? 1 : no_match;
}

#endif
