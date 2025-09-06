// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/physics/physobj.h>
#include "pragma/game/s_game.h"
#include <sharedutils/util_string.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.server.entities.components.triggers.teleport;

using namespace pragma;

extern SGame *s_game;

void STriggerTeleportComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerTeleport::Initialize()
{
	TriggerTouch::Initialize();
	AddComponent<STriggerTeleportComponent>();
}
