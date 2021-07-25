/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GAME_OBJECT_POLICY_HPP__
#define __GAME_OBJECT_POLICY_HPP__

#include <luabind/detail/policy.hpp>

namespace luabind {
	namespace detail {

		struct game_object_converter
		{
			template <class T>
			void to_lua(lua_State* L, T const& x)
			{
				auto &xnc = const_cast<T&>(x);
				if constexpr(std::is_pointer_v<decltype(xnc.GetLuaObject())>)
					xnc.GetLuaObject()->push(L);
				else
					xnc.GetLuaObject().push(L);
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

		struct game_object_converter_smartptr
		{
			template <class T>
			void to_lua(lua_State* L, T const& x)
			{
				const_cast<T&>(x).get()->GetLuaObject().push(L);
			}

			template <class T>
			void to_lua(lua_State* L, T* x)
			{
				if(!x || !x->get())
					lua_pushnil(L);
				else
					to_lua(*x);
			}
		};

		struct game_object_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Game object policy only supports cpp -> lua");
				using type = game_object_converter;
			};
		};
		struct game_object_smartptr_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Game object policy only supports cpp -> lua");
				using type = game_object_converter_smartptr;
			};
		};

	} // namespace detail

	template< unsigned int N >
	using game_object_policy = meta::type_list< converter_policy_injector< N, detail::game_object_policy > >;

	template< unsigned int N >
	using game_object_smartptr_policy = meta::type_list< converter_policy_injector< N, detail::game_object_smartptr_policy > >;

} // namespace luabind

#endif
