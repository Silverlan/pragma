// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"


export module pragma.shared:scripting.lua.converters.global_string;

export import :util.global_string_table;
export import luabind;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<pragma::GString> : native_converter_base<pragma::GString> {
		enum { consumed_args = 1 };

		template<typename U>
		pragma::GString to_cpp(lua_State *L, U u, int index);

		template<class U>
		static int match(lua_State *l, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}

		void to_lua(lua_State *L, pragma::GString const &x);
		void to_lua(lua_State *L, pragma::GString *x);
	  public:
		static value_type to_cpp_deferred(lua_State *, int) { return {}; }
		static void to_lua_deferred(lua_State *, param_type) {}
		static int compute_score(lua_State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const pragma::GString> : default_converter<pragma::GString> {};

	template<>
	struct DLLNETWORK default_converter<pragma::GString const &> : default_converter<pragma::GString> {};

	template<>
	struct DLLNETWORK default_converter<pragma::GString &&> : default_converter<pragma::GString> {};
}

export namespace luabind {
	template<typename U>
	pragma::GString default_converter<pragma::GString>::to_cpp(lua_State *L, U u, int index)
	{
		return {luaL_checkstring(L, index)};
	}

	template<class U>
	int default_converter<pragma::GString>::match(lua_State *l, U u, int index)
	{
		return lua_isstring(l, index) ? 1 : no_match;
	}
}
