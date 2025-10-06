// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.server.scripting.lua.libraries.ai;

export import pragma.server.ai.schedule;

export namespace Lua {
	namespace ai {
		namespace server {
			DLLSERVER std::shared_ptr<pragma::ai::BehaviorNode> create_lua_task(lua_State *l, luabind::object oClass, pragma::ai::BehaviorNode::Type taskType, pragma::ai::SelectorType selectorType);
			DLLSERVER void register_library(Lua::Interface &lua);
		};
	};
};
