// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/physics/physobj.h>
#include "pragma/physics/movetypes.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.prop_physics;

import pragma.client.entities.components.physics;

using namespace pragma;

LINK_ENTITY_TO_CLASS(prop_physics, CPropPhysics);

void CPropPhysicsComponent::Initialize() { BasePropPhysicsComponent::Initialize(); }

void CPropPhysicsComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePropPhysicsComponent::OnEntityComponentAdded(component);
	/*if(typeid(component) == typeid(pragma::CPhysicsComponent))
	{
		auto *pPhysComponent = static_cast<pragma::CPhysicsComponent*>(&component);
		pPhysComponent->SetMoveType(MOVETYPE::PHYSICS);
		// m_propPhysType = PHYSICSTYPE::DYNAMIC;
	}
	else if(typeid(component) == typeid(pragma::CRenderComponent))
	{
		auto *pRenderComponent = static_cast<pragma::CRenderComponent*>(&component);
		pRenderComponent->SetCastShadows(true);
	}*/
}

void CPropPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CPropPhysics::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPropPhysicsComponent>();
}
