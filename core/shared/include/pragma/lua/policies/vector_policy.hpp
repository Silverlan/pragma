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

		template<typename TValue=void,class Policies=luabind::no_policies>
		struct vector_converter
		{
			enum { consumed_args = 1 };

			template <class U>
			std::vector<TValue> to_cpp(lua_State* L, U u, int index)
			{
				specialized_converter_policy_n<0, Policies, TValue, lua_to_cpp> converter;

				auto o = luabind::object{luabind::from_stack(L,index)};
				auto n = Lua::GetObjectLength(L,o);
				std::vector<TValue> v;
				v.reserve(n);
				for(luabind::iterator it{o},end;it!=end;++it)
				{
					luabind::object o = *it;
					o.push(L);
					v.push_back(converter.to_cpp(L, decorate_type_t<TValue>(), -1));
					lua_pop(L,1);
				}
				return v;
			}

			template <class U>
			static int match(lua_State *l, U, int index)
			{
				return lua_istable(l,index) ? 0 : no_match;
			}

			template <class TValue>
			void to_lua(lua_State* L, std::vector<TValue> const& x)
			{
				specialized_converter_policy_n<0, Policies, TValue, cpp_to_lua> converter;
				auto t = luabind::newtable(L);
				t.push(L);
				int index = 1;

				for(const auto& element : x)
				{
					if constexpr(std::is_fundamental_v<decltype(element)>)
						t[index] = element;
					else
					{
						converter.to_lua(L, element);
						lua_rawseti(L, -2, index);
					}
					++index;
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

			template <class U>
			void converter_postcall(lua_State*, U u, int) {}
		};

		template<typename TValue=void,class Policies=luabind::no_policies>
		struct vector_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				using type = vector_converter<TValue,Policies>;
			};
		};

	} // namespace detail

	// array_policy
	template< unsigned int N,typename TValue=void,class Policies=luabind::no_policies >
	using vector_policy = meta::type_list< converter_policy_injector< N, detail::vector_policy<TValue,Policies> > >;

	namespace detail {
		template<typename TValue=void,uint32_t SIZE=0,class Policies=luabind::no_policies>
		struct array_converter
		{
			enum { consumed_args = 1 };

			template <class U>
			std::array<TValue,SIZE> to_cpp(lua_State* L, U u, int index)
			{
				specialized_converter_policy_n<0, Policies, TValue, lua_to_cpp> converter;

				auto o = luabind::object{luabind::from_stack(L,index)};
				auto n = Lua::GetObjectLength(L,o);
				assert(n == SIZE);
				if(n > SIZE)
					n = SIZE;
				uint32_t i = 0;
				std::array<TValue,SIZE> v;
				for(luabind::iterator it{o},end;it!=end && i < n;++it,++i)
				{
					luabind::object o = *it;
					o.push(L);
					v[i] = converter.to_cpp(L, decorate_type_t<TValue>(), -1);
					lua_pop(L,1);
				}
				return v;
			}

			template <class U>
			static int match(lua_State *l, U, int index)
			{
				if(!lua_istable(l,index))
					return no_match;
				auto n = Lua::GetObjectLength(l,luabind::object{luabind::from_stack{l,index}});
				if(n != SIZE)
					return no_match;
				return 0;
			}

			template <class TValue>
			void to_lua(lua_State* L, std::array<TValue,SIZE> const& x)
			{
				specialized_converter_policy_n<0, Policies, TValue, cpp_to_lua> converter;
				auto t = luabind::newtable(L);
				t.push(L);
				int index = 1;

				for(const auto& element : x)
				{
					if constexpr(std::is_fundamental_v<decltype(element)>)
						t[index] = element;
					else
					{
						converter.to_lua(L, element);
						lua_rawseti(L, -2, index);
					}
					++index;
				}
			}

			template <class T>
			void to_lua(lua_State* L, std::array<TValue,SIZE>* x)
			{
				if(!x)
					luabind::newtable(L).push(L);
				else
					to_lua(L,*x);
			}

			template <class U>
			void converter_postcall(lua_State*, U u, int) {}
		};

		template<typename TValue=void,uint32_t SIZE=0,class Policies=luabind::no_policies>
		struct array_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				using type = array_converter<TValue,SIZE,Policies>;
			};
		};

	} // namespace detail

	template< unsigned int N,typename TValue=void,uint32_t SIZE=0,class Policies=luabind::no_policies >
	using array_policy = meta::type_list< converter_policy_injector< N, detail::array_policy<TValue,SIZE,Policies> > >;

} // namespace luabind

#endif
