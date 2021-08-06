/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_OPTIONAL_CONVERTER_T_HPP__
#define __LUA_OPTIONAL_CONVERTER_T_HPP__

#include "pragma/lua/converters/optional_converter.hpp"

template<typename T>
template <typename U>
std::optional<T> luabind::default_converter<std::optional<T>>::to_cpp(lua_State* L, U u, int index)
{
	if(lua_isnil(L,index))
		return {};
	return m_converter.to_cpp(L,decorate_type_t<T>(),index);
}

template<typename T>
template <class U>
int luabind::default_converter<std::optional<T>>::match(lua_State *l, U u, int index)
{
	if(lua_isnil(l,index))
		return 0;
	return m_converter.match(l,decorate_type_t<T>(),index);
}

template<typename T>
void luabind::default_converter<std::optional<T>>::to_lua(lua_State* L, std::optional<T> const& x)
{
	if(!x.has_value())
		lua_pushnil(L);
	else
		m_converter.to_lua(L,*x);
}

template<typename T>
void luabind::default_converter<std::optional<T>>::to_lua(lua_State* L, std::optional<T>* x)
{
	if(!x || !x->has_value())
		lua_pushnil(L);
	else
		to_lua(L,*x);
}

#endif
