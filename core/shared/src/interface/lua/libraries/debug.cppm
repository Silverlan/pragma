// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/components/base_ai_component.hpp"
#include <pragma/lua/luaapi.h>

export module pragma.client:scripting.lua.libraries.debug;

export namespace Lua {
	namespace debug {
		DLLNETWORK void stackdump(lua_State *l);
		DLLNETWORK int collectgarbage(lua_State *l);
		DLLNETWORK std::string move_state_to_string(lua_State *l, pragma::BaseAIComponent::MoveResult v);
		DLLNETWORK void beep(lua_State *l);
		DLLNETWORK void enable_remote_debugging(lua_State *l);
	};
};
