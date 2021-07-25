/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUAAPI_H__
#define __LUAAPI_H__

#include "pragma/definitions.h"
// Has to be included before luabind!
#include "lua_handles.hpp"

#include <luasystem.h>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>

namespace luabind
{
	// Additional types for overload resolution
	namespace adl {
		template <typename T>
		struct optional : object
		{
			optional(const object &o)
				: object(o)
			{}
		};

		template <typename T>
		struct tableT : object
		{
			tableT(const object &o)
				: object(o)
			{}
		};

		template <typename T,typename T2>
		struct tableTT : object
		{
			tableTT(const object &o)
				: object(o)
			{}
		};

		template <typename ...T>
		struct variant : object
		{
			variant(const object &o)
				: object(o)
			{}
		};

		template <typename ...T>
			struct mult : object
		{
			mult(const object &o)
				: object(o)
			{}
			mult(lua_State *l,T... args)
				: object()
			{
				([&] (auto & input)
				{
					Lua::Push(l,input);
				} (args), ...);
			}
		};

	} // namespace adl

	using adl::optional;
	using adl::tableT;
	using adl::tableTT;
	using adl::variant;
	using adl::mult;

	template <typename T>
	struct lua_proxy_traits<adl::optional<T> >
		: lua_proxy_traits<object>
	{
		static bool check(lua_State* L, int idx)
		{
			return lua_proxy_traits<object>::check(L, idx);
		}
	};

	template <typename T>
	struct lua_proxy_traits<adl::tableT<T> >
		: lua_proxy_traits<object>
	{
		static bool check(lua_State* L, int idx)
		{
			return lua_proxy_traits<object>::check(L, idx);
		}
	};

	template <typename T,typename T2>
	struct lua_proxy_traits<adl::tableTT<T,T2> >
		: lua_proxy_traits<object>
	{
		static bool check(lua_State* L, int idx)
		{
			return lua_proxy_traits<object>::check(L, idx);
		}
	};

	template <typename ...T>
	struct lua_proxy_traits<adl::variant<T...> >
		: lua_proxy_traits<object>
	{
		static bool check(lua_State* L, int idx)
		{
			return lua_proxy_traits<object>::check(L, idx);
		}
	};

	template <typename ...T>
	struct lua_proxy_traits<adl::mult<T...> >
		: lua_proxy_traits<object>
	{
		static bool check(lua_State* L, int idx)
		{
			return lua_proxy_traits<object>::check(L, idx);
		}
	};
};

namespace Lua
{
	extern const luabind::object nil;
	using object = luabind::object;

	class Interface;
	DLLNETWORK void initialize_lua_state(Lua::Interface &lua);
	DLLNETWORK void set_extended_lua_modules_enabled(bool b);
	DLLNETWORK bool get_extended_lua_modules_enabled();
};

#endif
