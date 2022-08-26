/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LTABLE_H__
#define __LTABLE_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace table
	{
		DLLNETWORK int has_value(lua_State *l);
		DLLNETWORK int random(lua_State *l);
		DLLNETWORK int is_empty(lua_State *l);
		DLLNETWORK int count(lua_State *l);
	};
};

#endif