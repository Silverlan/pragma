// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LGLOBAL_H__
#define __C_LGLOBAL_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua {
	DLLCLIENT double ServerTime(lua_State *l);
	DLLCLIENT double FrameTime(lua_State *l);

	DLLCLIENT void register_shared_client_state(lua_State *l);
};

#endif
