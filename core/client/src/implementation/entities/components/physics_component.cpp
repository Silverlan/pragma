// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <sharedutils/scope_guard.h>

module pragma.client;


import :entities.components.physics;
import :client_state;
import :game;
import :model;

using namespace pragma;


void CPhysicsComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { BasePhysicsComponent::RegisterEvents(componentManager, registerEvent); }
void CPhysicsComponent::Initialize()
{
	BasePhysicsComponent::Initialize();

#if 0
	// TODO
	BindEvent(CAnimatedComponent::EVENT_ON_SKELETON_UPDATED,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto *phys = GetPhysicsObject();
		if(phys != nullptr && GetPhysicsType() == PHYSICSTYPE::DYNAMIC)
		{
			auto *o = phys->GetCollisionObject();
			if(o != nullptr)
			{
				auto boneId = o->GetBoneID();
				Vector3 posRoot {0.f,0.f,0.f};
				auto animComponent = GetEntity().GetAnimatedComponent();
				if(animComponent.valid())
					animComponent->GetLocalBonePosition(boneId,posRoot);
				//offset = glm::translate(offset,-posRoot); // Deprecated? TODO: Remove this entire block!
				static_cast<CEOnSkeletonUpdated&>(evData.get()).physRootBoneId = boneId;
				return util::EventReply::Handled;
			}
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(CAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED,[this](std::reference_wrapper<ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto mdlComponent = ent.GetModelComponent();
		auto animComponent = ent.GetAnimatedComponent();
		if(mdlComponent.expired() || animComponent.expired())
			return;
		auto mdl = mdlComponent->GetModel();
		if(mdl != nullptr && static_cast<CAnimatedComponent&>(*animComponent).GetBoneMatrices().empty() == false)
			UpdateRagdollPose();
	});
#endif
}
void CPhysicsComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BasePhysicsComponent)); }
void CPhysicsComponent::OnEntitySpawn()
{
	BasePhysicsComponent::OnEntitySpawn();
	if(m_physicsType != PHYSICSTYPE::NONE)
		InitializePhysics(m_physicsType);
}
void CPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CPhysicsComponent::PrePhysicsSimulate()
{
	auto dt = pragma::get_cgame()->DeltaTime();
	if(dt > 0.0 && GetPhysicsType() != PHYSICSTYPE::SOFTBODY) {
		auto pVelComponent = GetEntity().GetComponent<pragma::VelocityComponent>();
		if(pVelComponent.valid())
			pVelComponent->SetVelocity(pVelComponent->GetVelocity() + GetLinearCorrectionVelocity() / static_cast<float>(dt));
	}
	BasePhysicsComponent::PrePhysicsSimulate();
}
bool CPhysicsComponent::PostPhysicsSimulate()
{
	auto dt = pragma::get_cgame()->DeltaTime();
	if(dt > 0.0 && GetPhysicsType() != PHYSICSTYPE::SOFTBODY) {
		auto pVelComponent = GetEntity().GetComponent<pragma::VelocityComponent>();
		if(pVelComponent.valid())
			pVelComponent->SetVelocity(pVelComponent->GetVelocity() - GetLinearCorrectionVelocity() / static_cast<float>(dt));
	}
	ResetLinearCorrectionVelocity();
	return BasePhysicsComponent::PostPhysicsSimulate();
}

void CPhysicsComponent::ReceiveData(NetPacket &packet)
{
	auto physType = static_cast<PHYSICSTYPE>(packet->Read<unsigned int>());
	m_physicsType = physType;
	auto moveType = static_cast<MOVETYPE>(packet->Read<uint32_t>());
	SetMoveType(moveType);
}

Bool CPhysicsComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetCollisionsEnabled)
		SetCollisionsEnabled(packet->Read<bool>());
	else if(eventId == m_netEvSetSimEnabled)
		SetSimulationEnabled(packet->Read<bool>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

void CPhysicsComponent::OnWake() { BasePhysicsComponent::OnWake(); }
void CPhysicsComponent::OnSleep() { BasePhysicsComponent::OnSleep(); }
