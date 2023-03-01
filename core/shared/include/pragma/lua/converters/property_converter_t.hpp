/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
