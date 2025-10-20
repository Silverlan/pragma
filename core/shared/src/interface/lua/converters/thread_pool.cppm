// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <luabind/detail/conversion_policies/native_converter.hpp>

export module pragma.shared:scripting.lua.converters.thread_pool;

export import :scripting.lua.classes.thread_pool;
export import :scripting.lua.core;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<pragma::lua::LuaThreadWrapper> : native_converter_base<pragma::lua::LuaThreadWrapper> {
		enum { consumed_args = 1 };

		template<typename U>
		pragma::lua::LuaThreadWrapper to_cpp(lua_State *L, U u, int index);

		template<class U>
		static int match(lua_State *l, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}

		void to_lua(lua_State *L, pragma::lua::LuaThreadWrapper const &x);
		void to_lua(lua_State *L, pragma::lua::LuaThreadWrapper *x);
	  public:
		static value_type to_cpp_deferred(lua_State *, int) { return {}; }
		static void to_lua_deferred(lua_State *, param_type) {}
		static int compute_score(lua_State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const pragma::lua::LuaThreadWrapper> : default_converter<pragma::lua::LuaThreadWrapper> {};

	template<>
	struct DLLNETWORK default_converter<pragma::lua::LuaThreadWrapper const &> : default_converter<pragma::lua::LuaThreadWrapper> {};

	template<>
	struct DLLNETWORK default_converter<pragma::lua::LuaThreadWrapper &&> : default_converter<pragma::lua::LuaThreadWrapper> {};
}

export {
	template<typename U>
	pragma::lua::LuaThreadWrapper luabind::default_converter<pragma::lua::LuaThreadWrapper>::to_cpp(lua_State *L, U u, int index)
	{
		if(Lua::IsType<pragma::lua::LuaThreadTask>(L, index))
			return pragma::lua::LuaThreadWrapper {Lua::Check<std::shared_ptr<pragma::lua::LuaThreadTask>>(L, index)};
		return pragma::lua::LuaThreadWrapper {Lua::Check<pragma::lua::LuaThreadPool>(L, index)};
	}

	template<class U>
	int luabind::default_converter<pragma::lua::LuaThreadWrapper>::match(lua_State *l, U u, int index)
	{
		return Lua::IsType<pragma::lua::LuaThreadTask>(l, index) ? 1 : Lua::IsType<pragma::lua::LuaThreadPool>(l, index) ? 1 : no_match;
	}
}
