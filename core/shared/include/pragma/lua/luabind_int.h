/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUABIND_INT_H__
#define __LUABIND_INT_H__

#include <pragma/lua/luaapi.h>

namespace luabind {
	// uint64_t
	template<>
	struct default_converter<uint64_t> : native_converter_base<uint64_t> {
		static int compute_score(lua_State *l, int index) { return lua_type(l, index) == LUA_TNUMBER ? 0 : -1; }

		uint64_t to_cpp_deferred(lua_State *l, int index) { return static_cast<uint64_t>(lua_tonumber(l, index)); }

		void to_lua_deferred(lua_State *l, uint64_t value) { lua_pushnumber(l, static_cast<lua_Number>(value)); }
	};

	template<>
	struct default_converter<uint64_t const> : default_converter<uint64_t> {};

	template<>
	struct default_converter<uint64_t const &> : default_converter<uint64_t> {};

	// int64_t
	template<>
	struct default_converter<int64_t> : native_converter_base<int64_t> {
		static int compute_score(lua_State *l, int index) { return lua_type(l, index) == LUA_TNUMBER ? 0 : -1; }

		int64_t to_cpp_deferred(lua_State *l, int index) { return static_cast<int64_t>(lua_tonumber(l, index)); }

		void to_lua_deferred(lua_State *l, int64_t value) { lua_pushnumber(l, static_cast<lua_Number>(value)); }
	};

	template<>
	struct default_converter<int64_t const> : default_converter<int64_t> {};

	template<>
	struct default_converter<int64_t const &> : default_converter<int64_t> {};
}

#endif
