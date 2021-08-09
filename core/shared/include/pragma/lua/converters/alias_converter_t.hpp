/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_ALIAS_CONVERTER_T_HPP__
#define __LUA_ALIAS_CONVERTER_T_HPP__

#include "pragma/lua/converters/alias_converter.hpp"

template <class TBase,class ...T>
template<size_t I, typename... Tp>
bool luabind::alias_converter<TBase,T...>::match_any(lua_State *L,int index)
{
	using T = std::tuple_element<I, std::tuple<Tp...> >::type;
	if(m_converter.match(L,decorate_type_t<copy_qualifiers_t<TBase,T>>(),index) != no_match)
	{
		m_tmp = std::make_unique<UNDERLYING_TYPE>();
		detail::AliasTypeConverter<UNDERLYING_TYPE,T>::convert(m_converter.to_cpp(L,decorate_type_t<copy_qualifiers_t<TBase,T>>(),index),*m_tmp);
		return true;
	}
	if constexpr(I+1 != sizeof...(Tp))
		return match_any<I +1,Tp...>(L,index);
	return false;
}

template <class TBase,class ...T>
template <class U>
int luabind::alias_converter<TBase,T...>::match(lua_State* L, U u, int index)
{
	auto res = m_converter.match(L,decorate_type_t<TBase>(),index);
	if(res != no_match)
		return 0;
	auto hasMatch = match_any<0,T...>(L,index);
	return hasMatch ? 1 : no_match;
}

template <class TBase,class ...T>
template <class U>
TBase luabind::alias_converter<TBase,T...>::to_cpp(lua_State* L, U u, int index)
{
	if(m_tmp)
		return *m_tmp;
	return m_converter.to_cpp(L,decorate_type_t<TBase>(),index);
}

template <class TBase,class ...T>
void luabind::alias_converter<TBase,T...>::to_lua(lua_State* L, TBase x)
{
	m_converter.to_lua(L,x);
}

#endif
