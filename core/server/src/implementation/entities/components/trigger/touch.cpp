// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <pragma/physics/collisionmasks.h>
#include "pragma/entities/trigger/trigger_spawnflags.h"
#include <sharedutils/util_string.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.server.entities.components.triggers.touch;

using namespace pragma;

LINK_ENTITY_TO_CLASS(trigger_touch, TriggerTouch);

void STouchComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerTouch::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STouchComponent>();
}
