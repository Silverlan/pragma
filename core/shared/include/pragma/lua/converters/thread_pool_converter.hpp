/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_THREAD_POOL_CONVERTER_HPP__
#define __LUA_THREAD_POOL_CONVERTER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/classes/thread_pool.hpp"
#include <luabind/detail/conversion_policies/native_converter.hpp>

namespace luabind {
	template<>
	struct DLLNETWORK default_converter<pragma::lua::LuaThreadWrapper>
		: native_converter_base<pragma::lua::LuaThreadWrapper>
	{
		enum { consumed_args = 1 };

		template <typename U>
		pragma::lua::LuaThreadWrapper to_cpp(lua_State* L, U u, int index);

		template <class U>
		static int match(lua_State *l, U u, int index);

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
		
		void to_lua(lua_State* L, pragma::lua::LuaThreadWrapper const& x);
		void to_lua(lua_State* L, pragma::lua::LuaThreadWrapper* x);
	public:
		static value_type to_cpp_deferred(lua_State*,int) {return {};}
		static void to_lua_deferred(lua_State*,param_type) {}
		static int compute_score(lua_State*,int) {return no_match;}
	};

	template<>
	struct DLLNETWORK default_converter< const pragma::lua::LuaThreadWrapper >
		: default_converter< pragma::lua::LuaThreadWrapper >
	{ };

	template<>
	struct DLLNETWORK default_converter<pragma::lua::LuaThreadWrapper const&>
		: default_converter<pragma::lua::LuaThreadWrapper>
	{};

	template<>
	struct DLLNETWORK default_converter<pragma::lua::LuaThreadWrapper&&>
		: default_converter<pragma::lua::LuaThreadWrapper>
	{};
}

#endif
