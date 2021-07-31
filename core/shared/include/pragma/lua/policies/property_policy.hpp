/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PROPERTY_POLICY_HPP__
#define __PROPERTY_POLICY_HPP__

#include <luabind/detail/policy.hpp>
#include "pragma/lua/classes/lproperty.hpp"

namespace luabind {
	namespace detail {

		struct property_converter
		{
			template <class T>
			void to_lua(lua_State* L, T const& x)
			{
				Lua::Property::push(L,*const_cast<T&>(x));
			}

			template <class T>
			void to_lua(lua_State* L, T* x)
			{
				if(!x)
					lua_pushnil(L);
				else
					to_lua(L,*x);
			}
		};

		struct property_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Property policy only supports cpp -> lua");
				using type = property_converter;
			};
		};

	} // namespace detail

	template< unsigned int N >
	using property_policy = meta::type_list< converter_policy_injector< N, detail::property_policy > >;

} // namespace luabind

#endif
