/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __VECTOR_POLICY_HPP__
#define __VECTOR_POLICY_HPP__

#include <luabind/detail/policy.hpp>
#include "pragma/lua/ldefinitions.h"

namespace luabind {
	namespace detail {

		template<typename TValue=void,luabind::object(*F)(const TValue&)=nullptr>
		struct vector_converter
		{
			template <class TValue>
			void to_lua(lua_State* L, std::vector<TValue> const& x)
			{
				auto t = luabind::newtable(L);
				if constexpr(!F)
				{
					uint32_t idx = 1;
					for(auto &v : x)
						t[idx++] = v;
				}
				else
				{
					uint32_t idx = 1;
					for(auto &v : x)
						t[idx++] = F(v);
				}
			}

			template <class T>
			void to_lua(lua_State* L, std::vector<TValue>* x)
			{
				if(!x)
					luabind::newtable(L).push(L);
				else
					to_lua(L,*x);
			}
		};

		template<typename TValue=void,luabind::object(*F)(const TValue&)=nullptr>
		struct vector_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Vector policy only supports cpp -> lua");
				using type = vector_converter<TValue,F>;
			};
		};

	} // namespace detail

	template< unsigned int N,typename TValue=void,luabind::object(*F)(const TValue&)=nullptr >
	using vector_policy = meta::type_list< converter_policy_injector< N, detail::vector_policy<TValue,F> > >;

} // namespace luabind

#endif
