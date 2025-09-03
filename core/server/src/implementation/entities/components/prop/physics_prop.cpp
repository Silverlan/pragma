// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include "pragma/physics/movetypes.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.prop.physics;

using namespace pragma;

LINK_ENTITY_TO_CLASS(prop_physics, PropPhysics);

void SPropPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PropPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPropPhysicsComponent>();
}
