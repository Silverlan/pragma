/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __TUPLE_POLICY_HPP__
#define __TUPLE_POLICY_HPP__

#include <luabind/detail/policy.hpp>

namespace luabind {
	namespace detail {

		struct tuple_converter
		{
			template <class ...T>
			void to_lua(lua_State* L, std::tuple<T...> const& x)
			{
				std::apply([&](auto&&... args) {(default_converter<decltype(args)>().to_lua(L,args), ...);}, x);
			}

			template <class ...T>
			void to_lua(lua_State* L, std::tuple<T...>* x)
			{
				if(!x)
					lua_pushnil(L);
				else
					to_lua(L,*x);
			}
		};

		struct tuple_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Tuple policy only supports cpp -> lua");
				using type = tuple_converter;
			};
		};

	} // namespace detail

	template< unsigned int N >
	using tuple_policy = meta::type_list< converter_policy_injector< N, detail::tuple_policy > >;

} // namespace luabind

#endif
