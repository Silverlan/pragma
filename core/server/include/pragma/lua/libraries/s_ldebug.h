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
