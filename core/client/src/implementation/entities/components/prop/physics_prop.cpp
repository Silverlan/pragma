// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.prop_physics;
import :entities.components.physics;

using namespace pragma;

void CPropPhysicsComponent::Initialize() { BasePropPhysicsComponent::Initialize(); }

void CPropPhysicsComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePropPhysicsComponent::OnEntityComponentAdded(component);
	/*if(typeid(component) == typeid(pragma::CPhysicsComponent))
	{
		auto *pPhysComponent = static_cast<pragma::CPhysicsComponent*>(&component);
		pPhysComponent->SetMoveType(pragma::physics::MoveType::Physics);
		// m_propPhysType = pragma::physics::PhysicsType::Dynamic;
	}
	else if(typeid(component) == typeid(pragma::CRenderComponent))
	{
		auto *pRenderComponent = static_cast<pragma::CRenderComponent*>(&component);
		pRenderComponent->SetCastShadows(true);
	}*/
}

void CPropPhysicsComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CPropPhysics::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPropPhysicsComponent>();
}
