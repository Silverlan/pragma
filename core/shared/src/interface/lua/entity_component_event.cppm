// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <vector>

export module pragma.shared:scripting.lua.entity_component_event;

export import :entities.components.events.event;

export struct DLLNETWORK LuaComponentEvent : public pragma::ComponentEvent {
	std::vector<luabind::object> arguments;
	virtual void PushArguments(lua_State *l) override;
};
