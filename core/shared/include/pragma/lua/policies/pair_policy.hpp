/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PAIR_POLICY_HPP__
#define __PAIR_POLICY_HPP__

#include <luabind/detail/policy.hpp>

namespace luabind {
	namespace detail {

		struct pair_converter
		{
			template <class T,class T2>
			void to_lua(lua_State* L, std::pair<T,T2> const& x)
			{
				default_converter<T>().to_lua(L,x.first);
				default_converter<T2>().to_lua(L,x.second);
			}

			template <class T,class T2>
			void to_lua(lua_State* L, std::pair<T,T2>* x)
			{
				if(!x)
					lua_pushnil(L);
				else
					to_lua(L,*x);
			}
		};

		struct pair_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Pair policy only supports cpp -> lua");
				using type = pair_converter;
			};
		};

	} // namespace detail

	template< unsigned int N >
	using pair_policy = meta::type_list< converter_policy_injector< N, detail::pair_policy > >;

} // namespace luabind

#endif
