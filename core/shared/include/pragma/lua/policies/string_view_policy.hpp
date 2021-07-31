/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __STRING_VIEW_POLICY_HPP__
#define __STRING_VIEW_POLICY_HPP__

#include <luabind/detail/policy.hpp>
#include <sharedutils/util.h>
#include <sharedutils/util_shared_handle.hpp>

namespace luabind {
	namespace detail {

		struct string_view_converter
		{
			void to_lua(lua_State* L, std::string_view const& x)
			{
				luabind::object{L,std::string{x}}.push(L);
			}
		};

		struct string_view_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Game object policy only supports cpp -> lua");
				using type = string_view_converter;
			};
		};

	} // namespace detail

	template< unsigned int N >
	using string_view_policy = meta::type_list< converter_policy_injector< N, detail::string_view_policy > >;

} // namespace luabind

#endif
