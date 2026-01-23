// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.prop_dynamic;
import :entities.components.physics;
import :entities.components.render;

using namespace pragma;

void CPropDynamicComponent::Initialize() { BasePropDynamicComponent::Initialize(); }

void CPropDynamicComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePropDynamicComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CPhysicsComponent)) {
		auto *pPhysComponent = static_cast<CPhysicsComponent *>(&component);
		pPhysComponent->SetMoveType(physics::MoveType::None);
		// m_propPhysType = pragma::physics::PhysicsType::Dynamic;
	}
	else if(typeid(component) == typeid(CRenderComponent)) {
		auto *pRenderComponent = static_cast<CRenderComponent *>(&component);
		pRenderComponent->SetCastShadows(true);
	}
}
void CPropDynamicComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CPropDynamic::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPropDynamicComponent>();
}
