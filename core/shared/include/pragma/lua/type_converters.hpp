// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_TYPE_CONVERTERS_HPP__
#define __LUA_TYPE_CONVERTERS_HPP__

#include <luabind/detail/conversion_policies/conversion_policies.hpp>

namespace luabind {
	/*template <>
	struct default_converter<X>
		: native_converter_base<X>
	{
		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
		}

		X from(lua_State* L, int index)
		{
			return X(lua_tonumber(L, index));
		}

		void to(lua_State* L, X const& x)
		{
			lua_pushnumber(L, x.value);
		}
	};

	template <>
	struct default_converter<X const&>
		: default_converter<X>
	{};*/
}

#endif
