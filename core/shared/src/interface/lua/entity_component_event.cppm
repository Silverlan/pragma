// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.entity_component_event;

export import :entities.components.events.event;
export import luabind;
export import std;

export struct DLLNETWORK LuaComponentEvent : public pragma::ComponentEvent {
	std::vector<luabind::object> arguments;
	virtual void PushArguments(lua::State *l) override;
};
