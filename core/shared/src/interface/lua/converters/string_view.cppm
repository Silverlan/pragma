// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <string_view>

export module pragma.shared:scripting.lua.converters.string_view;

export import luabind;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<std::string_view> : native_converter_base<std::string_view> {
		enum { consumed_args = 1 };

		template<typename U>
		std::string_view to_cpp(lua_State *L, U u, int index);

		template<class U>
		static int match(lua_State *l, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}

		void to_lua(lua_State *L, std::string_view const &x);
		void to_lua(lua_State *L, std::string_view *x);
	  public:
		static value_type to_cpp_deferred(lua_State *, int) { return {}; }
		static void to_lua_deferred(lua_State *, param_type) {}
		static int compute_score(lua_State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const std::string_view> : default_converter<std::string_view> {};

	template<>
	struct DLLNETWORK default_converter<std::string_view const &> : default_converter<std::string_view> {};

	template<>
	struct DLLNETWORK default_converter<std::string_view &&> : default_converter<std::string_view> {};
}

export {
	template<typename U>
	std::string_view luabind::default_converter<std::string_view>::to_cpp(lua_State *L, U u, int index)
	{
		return {luaL_checkstring(L, index)};
	}

	template<class U>
	int luabind::default_converter<std::string_view>::match(lua_State *l, U u, int index)
	{
		return lua_isstring(l, index) ? 1 : no_match;
	}
}
