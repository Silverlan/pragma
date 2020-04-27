/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_LDEBUG_H__
#define __S_LDEBUG_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace debug
	{
		namespace Server
		{
			DLLSERVER int behavior_selector_type_to_string(lua_State *l);
			DLLSERVER int behavior_task_decorator_type_to_string(lua_State *l);
			DLLSERVER int behavior_task_result_to_string(lua_State *l);
			DLLSERVER int behavior_task_type_to_string(lua_State *l);
			DLLSERVER int disposition_to_string(lua_State *l);
			DLLSERVER int memory_type_to_string(lua_State *l);
			DLLSERVER int npc_state_to_string(lua_State *l);
			DLLSERVER int task_to_string(lua_State *l);
		};
	};
};

#endif
