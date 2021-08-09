/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_RAW_OBJECT_HPP__
#define __LUA_RAW_OBJECT_HPP__

#include <luabind/object.hpp>

namespace pragma::lua
{
	template<typename T,typename TPush=T>
		static luabind::object raw_object_to_luabind_object(lua_State *l,T v)
	{
		// Using the value_converter will prevent the default_converter from getting triggered, which would cause an infinite recursion here
		luabind::detail::value_converter c;
		c.to_lua<TPush>(l,static_cast<TPush>(v));
		auto o = luabind::object{luabind::from_stack(l,-1)};
		Lua::Pop(l,1);
		return o;
	}
};

#endif
