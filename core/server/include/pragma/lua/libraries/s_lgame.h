/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_LGAME_H__
#define __S_LGAME_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace game
	{
		namespace Server
		{
			DLLSERVER int set_gravity(lua_State *l);
			DLLSERVER int get_gravity(lua_State *l);
			DLLSERVER int load_model(lua_State *l);
			DLLSERVER int create_model(lua_State *l);
			DLLSERVER int load_map(lua_State *l);
			DLLSERVER int change_level(lua_State *l);
		};
	};
};

#endif
