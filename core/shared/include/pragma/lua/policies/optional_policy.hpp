/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __OPTIONAL_POLICY_HPP__
#define __OPTIONAL_POLICY_HPP__

// TODO: Remove this file
#if 0
#include <luabind/detail/policy.hpp>
#include <optional>

namespace luabind {
	namespace detail {

		template<class ValuePolicy=void>
		struct optional_converter
		{
			template <class T>
			void to_lua(lua_State* L, std::optional<T> const& x)
			{
				if(!x.has_value())
					lua_pushnil(L);
				else
				{
					if constexpr(std::is_same_v<ValuePolicy,void>)
						default_converter<T>().to_lua(L, *x);
					else
					{
						specialized_converter_policy_n<0, luabind::policy_list<ValuePolicy>, T, cpp_to_lua> converter;
						converter.to_lua(L,const_cast<T&>(*x));
					}
				}
			}

			template <class T>
			void to_lua(lua_State* L, std::shared_ptr<T> const& x)
			{
				if(!x)
					lua_pushnil(L);
				else
				{
					if constexpr(std::is_same_v<ValuePolicy,void>)
						default_converter<T>().to_lua(L, *x);
					else
					{
						specialized_converter_policy_n<0, luabind::policy_list<ValuePolicy>, T, cpp_to_lua> converter;
						converter.to_lua(L,const_cast<T&>(*x));
					}
				}
			}

			template <class T>
			void to_lua(lua_State* L, std::optional<T>* x)
			{
				if(!x || !x->has_value())
					lua_pushnil(L);
				else
					to_lua(L,*x);
			}

			template <class T> requires(std::is_pointer_v<T>)
			void to_lua(lua_State* L, T x)
			{
				if(!x)
					lua_pushnil(L);
				else
				{
					specialized_converter_policy_n<0, luabind::policy_list<ValuePolicy>, std::add_lvalue_reference_t<std::remove_pointer_t<T>>, cpp_to_lua> converter;
					converter.to_lua(L,*x);
				}
			}
		};

		template<class ValuePolicy=void>
		struct optional_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Optional policy only supports cpp -> lua");
				using type = optional_converter<ValuePolicy>;
			};
		};

	} // namespace detail

	template< unsigned int N,class ValuePolicy=void >
	using optional_policy = meta::type_list< converter_policy_injector< N, detail::optional_policy<ValuePolicy> > >;

} // namespace luabind
#endif

#endif
