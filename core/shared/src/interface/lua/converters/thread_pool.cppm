// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.converters.thread_pool;

export import :scripting.lua.classes.thread_pool;
export import :scripting.lua.core;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<pragma::LuaCore::LuaThreadWrapper> : native_converter_base<pragma::LuaCore::LuaThreadWrapper> {
		enum { consumed_args = 1 };

		template<typename U>
		pragma::LuaCore::LuaThreadWrapper to_cpp(lua::State *L, U u, int index);

		template<class U>
		static int match(lua::State *l, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, pragma::LuaCore::LuaThreadWrapper const &x);
		void to_lua(lua::State *L, pragma::LuaCore::LuaThreadWrapper *x);
	  public:
		static value_type to_cpp_deferred(lua::State *, int) { return {}; }
		static void to_lua_deferred(lua::State *, param_type) {}
		static int compute_score(lua::State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const pragma::LuaCore::LuaThreadWrapper> : default_converter<pragma::LuaCore::LuaThreadWrapper> {};

	template<>
	struct DLLNETWORK default_converter<pragma::LuaCore::LuaThreadWrapper const &> : default_converter<pragma::LuaCore::LuaThreadWrapper> {};

	template<>
	struct DLLNETWORK default_converter<pragma::LuaCore::LuaThreadWrapper &&> : default_converter<pragma::LuaCore::LuaThreadWrapper> {};
}

export namespace luabind {
	template<typename U>
	pragma::LuaCore::LuaThreadWrapper default_converter<pragma::LuaCore::LuaThreadWrapper>::to_cpp(lua::State *L, U u, int index)
	{
		if(Lua::IsType<pragma::LuaCore::LuaThreadTask>(L, index))
			return pragma::LuaCore::LuaThreadWrapper {Lua::Check<std::shared_ptr<pragma::LuaCore::LuaThreadTask>>(L, index)};
		return pragma::LuaCore::LuaThreadWrapper {Lua::Check<pragma::LuaCore::LuaThreadPool>(L, index)};
	}

	template<class U>
	int default_converter<pragma::LuaCore::LuaThreadWrapper>::match(lua::State *l, U u, int index)
	{
		return Lua::IsType<pragma::LuaCore::LuaThreadTask>(l, index) ? 1 : Lua::IsType<pragma::LuaCore::LuaThreadPool>(l, index) ? 1 : no_match;
	}
}
