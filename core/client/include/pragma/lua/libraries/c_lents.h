/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LENTS_H__
#define __C_LENTS_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace ents
	{
		namespace Client
		{
			DLLCLIENT int get_local_player(lua_State *l);
			DLLCLIENT int get_listener(lua_State *l);
			DLLCLIENT int get_view_model(lua_State *l);
			DLLCLIENT int get_view_body(lua_State *l);
			DLLCLIENT int get_instance_buffer(lua_State *l);
			DLLCLIENT int get_instance_bone_buffer(lua_State *l);
		};
	};
};

#endif
