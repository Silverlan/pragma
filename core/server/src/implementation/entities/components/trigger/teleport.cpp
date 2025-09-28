// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <sharedutils/util_string.h>
#include "sharedutils/netpacket.hpp"

module pragma.server.entities.components.triggers.teleport;

import pragma.server.game;

using namespace pragma;

void STriggerTeleportComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerTeleport::Initialize()
{
	TriggerTouch::Initialize();
	AddComponent<STriggerTeleportComponent>();
}
