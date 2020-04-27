/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LRESOURCE_H__
#define __LRESOURCE_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace resource
	{
		DLLSERVER int add_file(lua_State *l);
		DLLSERVER int add_lua_file(lua_State *l);
		DLLSERVER int get_list(lua_State *l);
	};
};

#endif
