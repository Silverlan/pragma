/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GENERIC_POLICY_HPP__
#define __GENERIC_POLICY_HPP__

#include <luabind/detail/policy.hpp>

namespace luabind {
	namespace detail {

		template<typename T,int(*TFuncMatch)(lua_State*,int),T(*TToCpp)(lua_State*,int),uint32_t TNumConsumed=1>
		struct generic_converter
		{
			enum { consumed_args = TNumConsumed };

			template <class U>
			T to_cpp(lua_State* L, U u, int index)
			{
				return TToCpp(L,index);
			}

			template <class U>
			static int match(lua_State *l, U, int index)
			{
				return TFuncMatch(l,index);
			}

			template <class U>
			void converter_postcall(lua_State*, U u, int) {}
		};

		template<typename TType,int(*TFuncMatch)(lua_State*,int),TType(*TToCpp)(lua_State*,int),uint32_t TNumConsumed=1>
		struct generic_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				using type = generic_converter<TType,TFuncMatch,TToCpp,TNumConsumed>;
			};
		};

	} // namespace detail

	template< unsigned int N,typename T,int(*TFuncMatch)(lua_State*,int),T(*TToCpp)(lua_State*,int),uint32_t TNumConsumed=1 >
	using generic_policy = meta::type_list< converter_policy_injector< N, detail::generic_policy<T,TFuncMatch,TToCpp,TNumConsumed> > >;
} // namespace luabind

#endif
