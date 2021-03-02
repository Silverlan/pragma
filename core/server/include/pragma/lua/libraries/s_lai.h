/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LAI_H__
#define __S_LAI_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/ai/ai_behavior.h"

namespace Lua
{
	namespace ai
	{
		namespace server
		{
			DLLSERVER std::shared_ptr<pragma::ai::BehaviorNode> create_lua_task(lua_State *l,luabind::object oClass,pragma::ai::BehaviorNode::Type taskType,pragma::ai::SelectorType selectorType);
			DLLSERVER void register_library(Lua::Interface &lua);
		};
	};
};

#endif

