/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_GUI_ELEMENT_CONVERTERS_T_HPP__
#define __LUA_GUI_ELEMENT_CONVERTERS_T_HPP__

#include "pragma/lua/converters/gui_element_converter.hpp"
#include "pragma/gui/wgui_luainterface.h"

template<typename T, typename TConverter>
template<class U>
T luabind::gui_element_converter<T, TConverter>::to_cpp(lua_State *L, U u, int index)
{
	return m_converter.to_cpp(L, u, index);
}

template<typename T, typename TConverter>
void luabind::gui_element_converter<T, TConverter>::to_lua(lua_State *L, T x)
{
	if constexpr(std::is_same_v<T, WIHandle>) {
		if(x.expired())
			lua_pushnil(L);
		else {
			auto &el = *x;
			auto o = WGUILuaInterface::GetLuaObject(L, const_cast<std::remove_cvref_t<decltype(el)> &>(el));
			o.push(L);
		}
	}
	else if constexpr(std::is_pointer_v<T>) {
		if(!x)
			lua_pushnil(L);
		else {
			auto o = WGUILuaInterface::GetLuaObject(L, *const_cast<T>(x));
			o.push(L);
		}
	}
	else {
		auto o = WGUILuaInterface::GetLuaObject(L, const_cast<T &>(x));
		o.push(L);
	}
}

template<typename T, typename TConverter>
template<class U>
int luabind::gui_element_converter<T, TConverter>::match(lua_State *L, U u, int index)
{
	return m_converter.match(L, u, index);
}

#endif
