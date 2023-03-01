/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __SHARED_FROM_THIS_POLICY_HPP__
#define __SHARED_FROM_THIS_POLICY_HPP__

#include <luabind/detail/policy.hpp>
#include <memory>

namespace luabind {
	namespace detail {

		struct shared_from_this_converter {
			template<class T>
			void to_lua(lua_State *L, T const &x)
			{
				default_converter<T>().to_lua(L, const_cast<T &>(x).shared_from_this());
			}

			template<class T>
			void to_lua(lua_State *L, T *x)
			{
				if(!x)
					lua_pushnil(L);
				else
					to_lua(L, *x);
			}
		};

		struct shared_from_this_policy {
			template<class T, class Direction>
			struct specialize {
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "shared_from_this policy only supports cpp -> lua");
				using type = shared_from_this_converter;
			};
		};

	} // namespace detail

	template<unsigned int N>
	using shared_from_this_policy = meta::type_list<converter_policy_injector<N, detail::shared_from_this_policy>>;

} // namespace luabind

#endif
