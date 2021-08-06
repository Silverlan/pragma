/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PAIR_POLICY_HPP__
#define __PAIR_POLICY_HPP__

// TODO: Remove this file
#if 0
#include <luabind/detail/policy.hpp>

namespace luabind {
	namespace detail {

		template<typename T0=void,typename T1=void,class ValuePolicy1=void,class ValuePolicy2=void>
		struct pair_converter
		{
			enum { consumed_args = 1 };

			template <typename U>
			std::pair<T0,T1> to_cpp(lua_State* L, U u, int index)
			{
				auto o = luabind::object{luabind::from_stack(L,index)};
				std::pair<T0,T1> pair {};
				luabind::iterator it {o};
				luabind::iterator end;
				if(it != end)
				{
					specialized_converter_policy_n<0, luabind::policy_list<ValuePolicy1>, T0, lua_to_cpp> converter;
					luabind::object o0 = *it;
					o0.push(L);
					pair.first = converter.to_cpp(L, decorate_type_t<T0>(), -1);
					lua_pop(L,1);

					++it;
					if(it != end)
					{
						specialized_converter_policy_n<0, luabind::policy_list<ValuePolicy2>, T1, lua_to_cpp> converter;
						luabind::object o1 = *it;
						o1.push(L);
						pair.second = converter.to_cpp(L, decorate_type_t<T1>(), -1);
						lua_pop(L,1);
					}
				}
				return pair;
			}

			template <class U>
			static int match(lua_State *l, U, int index)
			{
				return lua_istable(l,index) ? 0 : no_match;
			}

			template <class U>
			void converter_postcall(lua_State*, U u, int) {}

			template <class T,class T2>
			void to_lua(lua_State* L, std::pair<T,T2> const& x)
			{
				if constexpr(std::is_same_v<ValuePolicy1,void>)
					default_converter<T>().to_lua(L,x.first);
				else
				{
					specialized_converter_policy_n<0, luabind::policy_list<ValuePolicy1>, T, cpp_to_lua> converter;
					converter.to_lua(L,x.first);
				}

				if constexpr(std::is_same_v<ValuePolicy2,void>)
					default_converter<T2>().to_lua(L,x.second);
				else
				{
					specialized_converter_policy_n<0, luabind::policy_list<ValuePolicy2>, T2, cpp_to_lua> converter;
					converter.to_lua(L,x.second);
				}
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

		template<typename T0=void,typename T1=void,class ValuePolicy1=void,class ValuePolicy2=void>
		struct pair_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				using type = pair_converter<T0,T1,ValuePolicy1,ValuePolicy2>;
			};
		};

	} // namespace detail

	template< unsigned int N,typename T0=void,typename T1=void,class ValuePolicy1=void,class ValuePolicy2=void >
	using pair_policy = meta::type_list< converter_policy_injector< N, detail::pair_policy<T0,T1,ValuePolicy1,ValuePolicy2> > >;

} // namespace luabind
#endif

#endif
