/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_PAIR_CONVERTER_HPP__
#define __LUA_PAIR_CONVERTER_HPP__

#include <luabind/detail/conversion_policies/native_converter.hpp>

namespace luabind {
	template<typename T0, typename T1>
	struct default_converter<std::pair<T0, T1>> : native_converter_base<std::pair<T0, T1>> {
		enum { consumed_args = 2 };

		template<typename U>
		std::pair<T0, T1> to_cpp(lua_State *L, U u, int index);

		template<class U>
		int match(lua_State *l, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}

		void to_lua(lua_State *L, std::pair<T0, T1> const &x);
		void to_lua(lua_State *L, std::pair<T0, T1> *x);
	  private:
		default_converter<T0> c0;
		default_converter<T1> c1;
	};

	template<typename T0, typename T1>
	struct default_converter<const std::pair<T0, T1>> : default_converter<std::pair<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<std::pair<T0, T1> const &> : default_converter<std::pair<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<std::pair<T0, T1> &&> : default_converter<std::pair<T0, T1>> {};
}

namespace luabind {
	template<class... T>
	struct default_converter<std::tuple<T...>> : native_converter_base<std::tuple<T...>> {
		enum { consumed_args = sizeof...(T) };

		template<size_t I = 0, typename... Tp>
		int match_all(lua_State *L, int index, std::tuple<default_converter<Tp>...> &); // tuple parameter is unused but required for overload resolution for some reason

		template<size_t I = 0, typename... Tp>
		void to_lua_all(lua_State *L, const std::tuple<Tp...> &t);

		template<size_t I = 0, typename... Tp>
		void to_cpp_all(lua_State *L, int index, std::tuple<Tp...> &t);

		template<typename U>
		std::tuple<T...> to_cpp(lua_State *L, U u, int index);

		template<class U>
		int match(lua_State *l, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}

		void to_lua(lua_State *L, std::tuple<T...> const &x);
		void to_lua(lua_State *L, std::tuple<T...> *x);
	  private:
		std::tuple<default_converter<T>...> m_converters;
	};

	template<class... T>
	struct default_converter<const std::tuple<T...>> : default_converter<std::tuple<T...>> {};

	template<class... T>
	struct default_converter<std::tuple<T...> const &> : default_converter<std::tuple<T...>> {};

	template<class... T>
	struct default_converter<std::tuple<T...> &&> : default_converter<std::tuple<T...>> {};
}

#endif
