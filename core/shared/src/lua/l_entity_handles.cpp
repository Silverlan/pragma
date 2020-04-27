/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/l_entity_handles.hpp"

bool Lua::CheckComponentHandle(lua_State *l,const BaseEntityComponentHandle &handle)
{
	if(handle.expired())
	{
		Lua::PushString(l,"Attempted to use a NULL component handle");
		lua_error(l);
		return false;
	}
	return true;
}
