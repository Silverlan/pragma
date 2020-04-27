/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LUA_COMPONENT_EVENT_HPP__
#define __LUA_COMPONENT_EVENT_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/entity_component_event.hpp"
#include <pragma/lua/luaapi.h>
#include <vector>

struct DLLNETWORK LuaComponentEvent
	: public pragma::ComponentEvent
{
	std::vector<luabind::object> arguments;
	virtual void PushArguments(lua_State *l) override;
};

#endif
