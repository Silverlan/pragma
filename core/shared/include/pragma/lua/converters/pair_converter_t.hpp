/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_PAIR_CONVERTER_T_HPP__
#define __LUA_PAIR_CONVERTER_T_HPP__

#include "pragma/lua/converters/pair_converter.hpp"

template<typename T0, typename T1>
template<typename U>
std::pair<T0, T1> luabind::default_converter<std::pair<T0, T1>>::to_cpp(lua_State *L, U u, int index)
{
	std::pair<T0, T1> pair {};
	pair.first = c0.to_cpp(L, decorate_type_t<T0>(), index);
	pair.second = c1.to_cpp(L, decorate_type_t<T1>(), index + 1);
	return pair;
}

template<typename T0, typename T1>
template<class U>
int luabind::default_converter<std::pair<T0, T1>>::match(lua_State *l, U u, int index)
{
	return (c0.match(l, decorate_type_t<T0>(), index) == 0 && c1.match(l, decorate_type_t<T1>(), index + 1) == 0) ? 0 : no_match;
}

template<typename T0, typename T1>
void luabind::default_converter<std::pair<T0, T1>>::to_lua(lua_State *L, std::pair<T0, T1> const &x)
{
	c0.to_lua(L, x.first);
	c1.to_lua(L, x.second);
}

template<typename T0, typename T1>
void luabind::default_converter<std::pair<T0, T1>>::to_lua(lua_State *L, std::pair<T0, T1> *x)
{
	if(!x)
		lua_pushnil(L);
	else
		to_lua(L, *x);
}

template<class... T>
template<size_t I, typename... Tp>
int luabind::default_converter<std::tuple<T...>>::match_all(lua_State *L, int index, std::tuple<default_converter<Tp>...> &)
{                                                                       // tuple parameter is unused but required for overload resolution for some reason
	using T2 = typename std::tuple_element<I, std::tuple<Tp...>>::type; //HEY!
	if(std::get<I>(m_converters).match(L, decorate_type_t<base_type<T2>>(), index) != 0)
		return no_match;
	if constexpr(I + 1 != sizeof...(Tp))
		return match_all<I + 1>(L, index + 1, m_converters);
	return 0;
}

template<class... T>
template<size_t I, typename... Tp>
void luabind::default_converter<std::tuple<T...>>::to_lua_all(lua_State *L, const std::tuple<Tp...> &t)
{
	std::get<I>(m_converters).to_lua(L, std::get<I>(t));
	if constexpr(I + 1 != sizeof...(Tp))
		to_lua_all<I + 1>(L, t);
}

template<class... T>
template<size_t I, typename... Tp>
void luabind::default_converter<std::tuple<T...>>::to_cpp_all(lua_State *L, int index, std::tuple<Tp...> &t)
{
	auto &v = std::get<I>(t);
	v = std::get<I>(m_converters).to_cpp(L, decorate_type_t<base_type<decltype(v)>>(), index++);
	if constexpr(I + 1 != sizeof...(Tp))
		to_cpp_all<I + 1>(L, index, t);
}

template<class... T>
template<typename U>
std::tuple<T...> luabind::default_converter<std::tuple<T...>>::to_cpp(lua_State *L, U u, int index)
{
	std::tuple<T...> tuple {};
	to_cpp_all(L, index, tuple);
	return tuple;
}

template<class... T>
template<class U>
int luabind::default_converter<std::tuple<T...>>::match(lua_State *l, U u, int index)
{
	return match_all<0, T...>(l, index, m_converters);
}

template<class... T>
void luabind::default_converter<std::tuple<T...>>::to_lua(lua_State *L, std::tuple<T...> const &x)
{
	to_lua_all(L, x);
}

template<class... T>
void luabind::default_converter<std::tuple<T...>>::to_lua(lua_State *L, std::tuple<T...> *x)
{
	if(!x)
		lua_pushnil(L);
	else
		to_lua(L, *x);
}

#endif
