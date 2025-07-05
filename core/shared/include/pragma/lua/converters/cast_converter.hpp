// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_CAST_CONVERTER_HPP__
#define __LUA_CAST_CONVERTER_HPP__

#include "pragma/networkdefinitions.h"
#include <luabind/detail/conversion_policies/conversion_policies.hpp>

namespace luabind {
	template<typename TCpp, typename TLua>
	struct cast_converter : detail::default_converter_generator<TLua> {
		enum { consumed_args = 1 };

		template<class U>
		TCpp to_cpp(lua_State *L, U, int index);

		template<class U>
		int match(lua_State *, U, int index);

		template<class U>
		void converter_postcall(lua_State *, U, int)
		{
		}

		void to_lua(lua_State *L, TCpp v);
	};
};

#endif
