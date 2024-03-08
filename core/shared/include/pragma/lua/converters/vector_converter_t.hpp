/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_VECTOR_CONVERTER_T_HPP__
#define __LUA_VECTOR_CONVERTER_T_HPP__

#include "pragma/lua/converters/vector_converter.hpp"

namespace luabind {
	template<class T>
	struct is_unordered_map {
		static constexpr bool value = false;
	};

	template<class Key, class Value>
	struct is_unordered_map<std::unordered_map<Key, Value>> {
		static constexpr bool value = true;
	};
};

template<typename T>
template<typename U>
std::vector<T> luabind::default_converter<std::vector<T>>::to_cpp(lua_State *L, U u, int index)
{
	default_converter<T> converter;

	auto o = luabind::object {luabind::from_stack(L, index)};
	auto n = Lua::GetObjectLength(L, o);
	std::vector<T> v;
	v.reserve(n);
	for(luabind::iterator it {o}, end; it != end; ++it) {
		luabind::object o = *it;
		o.push(L);
		if(converter.match(L, decorate_type_t<T>(), -1) != no_match)
			v.push_back(converter.to_cpp(L, decorate_type_t<T>(), -1));
		lua_pop(L, 1);
	}
	return v;
}

template<typename T>
template<class U>
int luabind::default_converter<std::vector<T>>::match(lua_State *l, U u, int index)
{
	return lua_istable(l, index) ? 0 : no_match;
}

template<typename T>
void luabind::default_converter<std::vector<T>>::to_lua(lua_State *L, std::vector<T> const &x)
{
	default_converter<T> converter;
	auto t = luabind::newtable(L);
	t.push(L);
	int index = 1;

	for(const auto &element : x) {
		if constexpr(std::is_fundamental_v<decltype(element)>)
			t[index] = element;
		else {
			auto top = lua_gettop(L);
			converter.to_lua(L, element);

			auto num = lua_gettop(L) - top;
			if(num == 1)
				lua_rawseti(L, -2, index);
			else {
				// Multiple values, add as sub-table
				auto subTable = luabind::newtable(L);
				for(int i = 0; i < num; ++i)
					subTable[i + 1] = luabind::object {luabind::from_stack(L, -num + i)};
				lua_pop(L, num);
				subTable.push(L);
				lua_rawseti(L, -2, index); // Add the sub-table to the main table
			}
		}
		++index;
	}
}

template<typename T>
void luabind::default_converter<std::vector<T>>::to_lua(lua_State *L, std::vector<T> *x)
{
	if(!x)
		luabind::newtable(L).push(L);
	else
		to_lua(L, *x);
}

template<class TMap>
template<typename U>
TMap luabind::map_converter<TMap>::to_cpp(lua_State *L, U u, int index)
{
	default_converter<T0> converter0;
	default_converter<T1> converter1;

	auto o = luabind::object {luabind::from_stack(L, index)};
	auto n = Lua::GetObjectLength(L, o);
	TMap v;
	if constexpr(is_unordered_map<TMap>::value)
		v.reserve(n);
	for(luabind::iterator it {o}, end; it != end; ++it) {
		luabind::object oKey = it.key();
		luabind::object o = *it;

		oKey.push(L);
		o.push(L);
		if(converter0.match(L, decorate_type_t<T0>(), -2) != no_match && converter1.match(L, decorate_type_t<T1>(), -1) != no_match)
			v[converter0.to_cpp(L, decorate_type_t<T0>(), -2)] = converter1.to_cpp(L, decorate_type_t<T1>(), -1);
		lua_pop(L, 2);
	}
	return v;
}

template<class TMap>
template<class U>
int luabind::map_converter<TMap>::match(lua_State *l, U u, int index)
{
	return lua_istable(l, index) ? 0 : no_match;
}

template<class TMap>
void luabind::map_converter<TMap>::to_lua(lua_State *L, TMap const &x)
{
	default_converter<T0> converter0;
	default_converter<T1> converter1;
	auto t = luabind::newtable(L);
	t.push(L);
	for(const auto &pair : x) {
		if constexpr(std::is_fundamental_v<decltype(pair.first)>) {
			if constexpr(std::is_fundamental_v<decltype(pair.second)>)
				t[pair.first] = pair.second;
			else {
				converter1.to_lua(L, pair.second);
				t[pair.first] = luabind::object {luabind::from_stack(L, -1)};
				lua_pop(L, 1);
			}
		}
		else {
			converter0.to_lua(L, pair.first);
			converter1.to_lua(L, pair.second);
			lua_rawset(L, -3);
		}
	}
}

template<class TMap>
void luabind::map_converter<TMap>::to_lua(lua_State *L, TMap *x)
{
	if(!x)
		luabind::newtable(L).push(L);
	else
		to_lua(L, *x);
}

template<typename T, size_t SIZE>
template<typename U>
std::array<T, SIZE> luabind::default_converter<std::array<T, SIZE>>::to_cpp(lua_State *L, U u, int index)
{
	default_converter<T> converter;

	auto o = luabind::object {luabind::from_stack(L, index)};
	auto n = Lua::GetObjectLength(L, o);
	assert(n == SIZE);
	if(n > SIZE)
		n = SIZE;
	std::array<T, SIZE> v;
	uint32_t i = 0;
	for(luabind::iterator it {o}, end; it != end; ++it) {
		luabind::object o = *it;
		o.push(L);
		if(converter.match(L, decorate_type_t<T>(), -1) != no_match)
			v[i] = converter.to_cpp(L, decorate_type_t<T>(), -1);
		lua_pop(L, 1);

		++i;
	}
	return v;
}

template<typename T, size_t SIZE>
template<class U>
int luabind::default_converter<std::array<T, SIZE>>::match(lua_State *l, U u, int index)
{
	if(!lua_istable(l, index))
		return no_match;
	auto n = Lua::GetObjectLength(l, index);
	if(n != SIZE)
		return no_match;
	return 0;
}

template<typename T, size_t SIZE>
void luabind::default_converter<std::array<T, SIZE>>::to_lua(lua_State *L, std::array<T, SIZE> const &x)
{
	default_converter<T> converter;
	auto t = luabind::newtable(L);
	t.push(L);
	int index = 1;

	for(const auto &element : x) {
		if constexpr(std::is_fundamental_v<decltype(element)>)
			t[index] = element;
		else {
			converter.to_lua(L, element);
			lua_rawseti(L, -2, index);
		}
		++index;
	}
}

template<typename T, size_t SIZE>
void luabind::default_converter<std::array<T, SIZE>>::to_lua(lua_State *L, std::array<T, SIZE> *x)
{
	if(!x)
		luabind::newtable(L).push(L);
	else
		to_lua(L, *x);
}

#endif
