/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_VECTOR_CONVERTER_HPP__
#define __LUA_VECTOR_CONVERTER_HPP__

#include <luabind/detail/conversion_policies/native_converter.hpp>
#include <vector>
#include <map>
#include <array>
#include <unordered_map>

namespace luabind {
	template<typename T>
	struct default_converter<std::vector<T>>
		: native_converter_base<std::vector<T>>
	{
		enum { consumed_args = 1 };

		template <typename U>
		std::vector<T> to_cpp(lua_State* L, U u, int index);

		template <class U>
		static int match(lua_State *l, U u, int index);

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
		
		void to_lua(lua_State* L, std::vector<T> const& x);
		void to_lua(lua_State* L, std::vector<T>* x);
	};

	template<typename T>
	struct default_converter< const std::vector<T> >
		: default_converter< std::vector<T> >
	{ };

	template<typename T>
	struct default_converter<std::vector<T> const&>
		: default_converter<std::vector<T>>
	{};

	template<typename T>
	struct default_converter<std::vector<T>&&>
		: default_converter<std::vector<T>>
	{};
}

namespace luabind {
	template<class TMap>
	struct map_converter
		: native_converter_base<TMap>
	{
		using T0 = TMap::key_type;
		using T1 = TMap::value_type::second_type;
		enum { consumed_args = 1 };

		template <typename U>
		TMap to_cpp(lua_State* L, U u, int index);

		template <class U>
		static int match(lua_State *l, U u, int index);

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
		
		void to_lua(lua_State* L, TMap const& x);
		void to_lua(lua_State* L, TMap* x);
	};

	// std::map
	template<typename T0,typename T1>
	struct default_converter<std::map<T0,T1>>
		: map_converter<std::map<T0,T1>>
	{};

	template<typename T0,typename T1>
	struct default_converter< const std::map<T0,T1> >
		: default_converter< std::map<T0,T1> >
	{ };

	template<typename T0,typename T1>
	struct default_converter<std::map<T0,T1> const&>
		: default_converter<std::map<T0,T1>>
	{};

	template<typename T0,typename T1>
	struct default_converter<std::map<T0,T1>&&>
		: default_converter<std::map<T0,T1>>
	{};

	// std::unordered_map
	template<typename T0,typename T1>
	struct default_converter<std::unordered_map<T0,T1>>
		: map_converter<std::unordered_map<T0,T1>>
	{};

	template<typename T0,typename T1>
	struct default_converter< const std::unordered_map<T0,T1> >
		: default_converter< std::unordered_map<T0,T1> >
	{ };

	template<typename T0,typename T1>
	struct default_converter<std::unordered_map<T0,T1> const&>
		: default_converter<std::unordered_map<T0,T1>>
	{};

	template<typename T0,typename T1>
	struct default_converter<std::unordered_map<T0,T1>&&>
		: default_converter<std::unordered_map<T0,T1>>
	{};
}

namespace luabind {
	template<typename T,size_t SIZE>
	struct default_converter<std::array<T,SIZE>>
		: native_converter_base<std::array<T,SIZE>>
	{
		enum { consumed_args = 1 };

		template <typename U>
		std::array<T,SIZE> to_cpp(lua_State* L, U u, int index);
		template <class U>
		static int match(lua_State *l, U u, int index);

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
		
		void to_lua(lua_State* L, std::array<T,SIZE> const& x);
		void to_lua(lua_State* L, std::array<T,SIZE>* x);
	};

	template<typename T,size_t SIZE>
	struct default_converter< const std::array<T,SIZE> >
		: default_converter< std::array<T,SIZE> >
	{ };

	template<typename T,size_t SIZE>
	struct default_converter<std::array<T,SIZE> const&>
		: default_converter<std::array<T,SIZE>>
	{};

	template<typename T,size_t SIZE>
	struct default_converter<std::array<T,SIZE>&&>
		: default_converter<std::array<T,SIZE>>
	{};
}

#endif
