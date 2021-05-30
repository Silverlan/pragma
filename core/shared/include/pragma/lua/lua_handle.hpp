/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_HANDLE_HPP__
#define __LUA_HANDLE_HPP__

struct lua_State;
namespace Lua
{
	template<class T>
		bool CheckWHandle(lua_State *l,const util::WeakHandle<T> &handle)
	{
		if(handle.expired())
		{
			Lua::PushString(l,"Attempted to use a NULL handle");
			lua_error(l);
			return false;
		}
		return true;
	}
};

#endif
