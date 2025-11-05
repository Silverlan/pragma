// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.entity_component_event;

void LuaComponentEvent::PushArguments(lua::State *l)
{
	for(auto &o : arguments)
		o.push(l);
}
