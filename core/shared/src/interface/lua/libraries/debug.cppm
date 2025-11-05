// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.debug;

export import :entities.components.base_ai;

export namespace Lua {
	namespace debug {
		DLLNETWORK void stackdump(lua::State *l);
		DLLNETWORK int collectgarbage(lua::State *l);
		DLLNETWORK std::string move_state_to_string(lua::State *l, pragma::BaseAIComponent::MoveResult v);
		DLLNETWORK void beep(lua::State *l);
		DLLNETWORK void enable_remote_debugging(lua::State *l);
	};
};
