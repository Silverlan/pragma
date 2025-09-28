// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_COMPONENT_EVENT_HPP__
#define __LUA_COMPONENT_EVENT_HPP__

#include <vector>

struct DLLNETWORK LuaComponentEvent : public pragma::ComponentEvent {
	std::vector<luabind::object> arguments;
	virtual void PushArguments(lua_State *l) override;
};

#endif
