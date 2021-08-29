/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_LUA_TYPES_UDM_HPP__
#define __PRAGMA_LUA_TYPES_UDM_HPP__

#include "pragma/definitions.h"
#include <udm.hpp>

#include <luasystem.h>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/pseudo_traits.hpp>
#include <luabind/make_function_signature.hpp>

namespace luabind
{
	// Additional types for overload resolution
	namespace adl {
		template <typename T>
		struct udm_generic : object
		{
			udm_generic(from_stack const& stack_reference)
				: object(stack_reference)
			{}
			udm_generic(const object &o)
				: object(o)
			{}
			udm_generic(lua_State *l,const T &t)
				: object(l,t)
			{}
			using value_type = T;
		};
	} // namespace adl

	namespace detail
	{
		template<typename T>
		struct pseudo_traits<adl::udm_generic<T>>
		{
			enum { is_variadic = false };
			using value_type = T;
		};
	};

	using adl::udm_generic;

	template <typename T>
	struct lua_proxy_traits<adl::udm_generic<T> >
		: lua_proxy_traits<object>
	{
		static bool check(lua_State* L, int idx)
		{
			if(!lua_proxy_traits<object>::check(L, idx))
				return false;
			//if(lua_isboolean(L,idx) || lua_isnumber(L,idx))
			//	return true;
			for(auto type : udm::GENERIC_TYPES)
			{
				auto r = udm::visit<false,true,false>(type,[](auto tag) {
					using T = decltype(tag)::type;
					if(luabind::object_cast_nothrow<T*>(luabind::from_stack{L,idx},static_cast<T*>(nullptr)) != nullptr)
						return true;
					return false;
				});
				if(r)
					return true;
			}
			return false;
		}
	};
};

namespace Lua
{
	using udm_generic = luabind::udm_generic<luabind::object>;
};

#endif
