// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_PROPERTY_CONVERTER_T_HPP__
#define __LUA_PROPERTY_CONVERTER_T_HPP__

#include "pragma/lua/converters/property_converter.hpp"
#include "pragma/lua/classes/lproperty.hpp"

// Note: This header should include method definitions from "property_converter.hpp", but due to a msvc compile error (21-08-08)
// this is currently not possible

template<typename T>
void luabind::push_property(lua_State *l, T &prop)
{
	Lua::Property::push(l, prop);
}

#endif
