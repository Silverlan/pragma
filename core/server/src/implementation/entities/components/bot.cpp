// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server.entities.components.bot;

import pragma.server.ai;

using namespace pragma;

void SBotComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void Bot::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SBotComponent>();
}
