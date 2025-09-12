// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/physics/physobj.h>
#include "pragma/physics/movetypes.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.prop_dynamic;

import pragma.client.entities.components.physics;
import pragma.client.entities.components.render;

using namespace pragma;

void CPropDynamicComponent::Initialize() { BasePropDynamicComponent::Initialize(); }

void CPropDynamicComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePropDynamicComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(pragma::CPhysicsComponent)) {
		auto *pPhysComponent = static_cast<pragma::CPhysicsComponent *>(&component);
		pPhysComponent->SetMoveType(MOVETYPE::NONE);
		// m_propPhysType = PHYSICSTYPE::DYNAMIC;
	}
	else if(typeid(component) == typeid(pragma::CRenderComponent)) {
		auto *pRenderComponent = static_cast<pragma::CRenderComponent *>(&component);
		pRenderComponent->SetCastShadows(true);
	}
}
void CPropDynamicComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CPropDynamic::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPropDynamicComponent>();
}
