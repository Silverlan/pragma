// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"


module pragma.server;
import :entities.components.bot;

import :ai;

using namespace pragma;

void SBotComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void Bot::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SBotComponent>();
}
