// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LTABLE_H__
#define __LTABLE_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua {
	namespace table {
		DLLNETWORK int has_value(lua_State *l);
		DLLNETWORK int random(lua_State *l);
		DLLNETWORK int is_empty(lua_State *l);
		DLLNETWORK int count(lua_State *l);
	};
};

#endif
