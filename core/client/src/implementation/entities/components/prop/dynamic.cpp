// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_client.h"

module pragma.client;


import :entities.components.prop_dynamic;
import :entities.components.physics;
import :entities.components.render;

using namespace pragma;

void CPropDynamicComponent::Initialize() { BasePropDynamicComponent::Initialize(); }

void CPropDynamicComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePropDynamicComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(pragma::CPhysicsComponent)) {
		auto *pPhysComponent = static_cast<pragma::CPhysicsComponent *>(&component);
		pPhysComponent->SetMoveType(pragma::physics::MOVETYPE::NONE);
		// m_propPhysType = pragma::physics::MOVETYPE::DYNAMIC;
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
