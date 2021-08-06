/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GUI_ELEMENT_POLICY_HPP__
#define __GUI_ELEMENT_POLICY_HPP__

#if 0
#include <luabind/detail/policy.hpp>
#include <sharedutils/util.h>
#include <sharedutils/util_shared_handle.hpp>
#include "pragma/gui/wgui_luainterface.h"

namespace luabind {
	namespace detail {

		struct gui_element_converter
		{
			template <class T>
			void to_lua(lua_State* L, T const& x)
			{
				if constexpr(std::is_same_v<T,WIHandle>)
				{
					if(x.expired())
						lua_pushnil(L);
					else
					{
						auto &el = *x;
						auto o = WGUILuaInterface::GetLuaObject(L,const_cast<std::remove_cvref_t<decltype(el)>&>(el));
						o.push(L);
					}
				}
				else
				{
					auto o = WGUILuaInterface::GetLuaObject(L,const_cast<T&>(x));
					o.push(L);
				}
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

		struct gui_element_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Game object policy only supports cpp -> lua");
				using type = gui_element_converter;
			};
		};

	} // namespace detail

	template< unsigned int N >
	using gui_element_policy = meta::type_list< converter_policy_injector< N, detail::gui_element_policy > >;
} // namespace luabind
#endif

#endif
