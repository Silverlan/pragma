// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components;

import :func_soft_physics;

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_physics_softbody, CFuncSoftPhysics);

void CFuncSoftPhysicsComponent::Initialize()
{
	BaseFuncSoftPhysicsComponent::Initialize();
	GetEntity().AddComponent("func_physics");
}
void CFuncSoftPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////

void CFuncSoftPhysics::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFuncSoftPhysicsComponent>();
}
