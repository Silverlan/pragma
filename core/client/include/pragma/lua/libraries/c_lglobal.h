/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LGLOBAL_H__
#define __C_LGLOBAL_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	DLLCLIENT double ServerTime(lua_State *l);
	DLLCLIENT double FrameTime(lua_State *l);

	DLLCLIENT void register_shared_client_state(lua_State *l);
};

#endif