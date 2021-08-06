/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __HANDLE_POLICY_HPP__
#define __HANDLE_POLICY_HPP__

// TODO: Remove this file
#if 0
#include <luabind/detail/policy.hpp>

namespace luabind {
	namespace detail {
		struct handle_converter
		{
			template <class T>
			void to_lua(lua_State* L, T const& x)
			{
				luabind::object{L,x.GetHandle()}.push(L);
			}

			template <class T>
			void to_lua(lua_State* L, T* x)
			{
				if(!x)
					lua_pushnil(L);
				else
					luabind::object{L,x->GetHandle()}.push(L);
			}
		};

		struct handle_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Handle policy only supports cpp -> lua");
				using type = handle_converter;
			};
		};
	} // namespace detail

	template< unsigned int N >
	using handle_policy = meta::type_list< converter_policy_injector< N, detail::handle_policy > >;

} // namespace luabind
#endif

#endif
