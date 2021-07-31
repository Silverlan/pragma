/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __FILE_POLICY_HPP__
#define __FILE_POLICY_HPP__

#include <luabind/detail/policy.hpp>
#include "pragma/lua/libraries/lfile.h"

namespace luabind {
	namespace detail {

		struct file_converter
		{
			enum { consumed_args = 0 };

			VFilePtr to_cpp(lua_State* L, const LFile &u, int)
			{
				return const_cast<LFile&>(u).GetHandle();
			}

			template <class U>
			static int match(lua_State *l, U, int index)
			{
				return Lua::IsType<LFile>(l,index);
			}
			
			template <class U>
			void converter_postcall(lua_State*, U u, int) {}
		};

		struct file_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, lua_to_cpp>::value, "File policy only supports lua -> cpp");
				using type = file_converter;
			};
		};

	} // namespace detail

	template< unsigned int N >
	using file_policy = meta::type_list< converter_policy_injector< N, detail::file_policy > >;

} // namespace luabind

#endif
