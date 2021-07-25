/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __OPTIONAL_POLICY_HPP__
#define __OPTIONAL_POLICY_HPP__

#include <luabind/detail/policy.hpp>
#include <optional>

namespace luabind {
	namespace detail {

		struct optional_converter
		{
			template <class T>
			void to_lua(lua_State* L, std::optional<T> const& x)
			{
				if(!x.has_value())
					lua_pushnil(L);
				else
					default_converter<T>().to_lua(L, *x);
			}

			template <class T>
			void to_lua(lua_State* L, std::optional<T>* x)
			{
				if(!x || !x->has_value())
					lua_pushnil(L);
				else
					to_lua(L,*x);
			}
		};

		struct optional_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Optional policy only supports cpp -> lua");
				using type = optional_converter;
			};
		};

	} // namespace detail

	template< unsigned int N >
	using optional_policy = meta::type_list< converter_policy_injector< N, detail::optional_policy > >;

} // namespace luabind

#endif
