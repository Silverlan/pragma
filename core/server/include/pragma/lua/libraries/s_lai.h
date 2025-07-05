// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_LAI_H__
#define __S_LAI_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/ai/ai_behavior.h"

namespace Lua {
	namespace ai {
		namespace server {
			DLLSERVER std::shared_ptr<pragma::ai::BehaviorNode> create_lua_task(lua_State *l, luabind::object oClass, pragma::ai::BehaviorNode::Type taskType, pragma::ai::SelectorType selectorType);
			DLLSERVER void register_library(Lua::Interface &lua);
		};
	};
};

#endif
