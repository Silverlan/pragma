/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_CAST_CONVERTER_T_HPP__
#define __LUA_CAST_CONVERTER_T_HPP__

#include "pragma/lua/converters/cast_converter.hpp"

template<typename TCpp, typename TLua>
template<class U>
TCpp luabind::cast_converter<TCpp, TLua>::to_cpp(lua_State *L, U u, int index)
{
	return static_cast<TCpp>(detail::default_converter_generator<TLua>::to_cpp(L, u, index));
}

template<typename TCpp, typename TLua>
template<class U>
int luabind::cast_converter<TCpp, TLua>::match(lua_State *l, U u, int index)
{
	return detail::default_converter_generator<TLua>::match(l, luabind::decorate_type_t<TLua> {}, index);
}

template<typename TCpp, typename TLua>
void luabind::cast_converter<TCpp, TLua>::to_lua(lua_State *L, TCpp v)
{
	detail::default_converter_generator<TLua>::to_lua(L, static_cast<TLua>(v));
}

#endif
