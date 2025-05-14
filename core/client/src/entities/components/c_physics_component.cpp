/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/c_modelmanager.h"
#include <pragma/audio/alsound_type.h>
#include <sharedutils/scope_guard.h>
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

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
	auto dt = c_game->DeltaTime();
	if(dt > 0.0 && GetPhysicsType() != PHYSICSTYPE::SOFTBODY) {
		auto pVelComponent = GetEntity().GetComponent<pragma::VelocityComponent>();
		if(pVelComponent.valid())
			pVelComponent->SetVelocity(pVelComponent->GetVelocity() + GetLinearCorrectionVelocity() / static_cast<float>(dt));
	}
	BasePhysicsComponent::PrePhysicsSimulate();
}
bool CPhysicsComponent::PostPhysicsSimulate()
{
	auto dt = c_game->DeltaTime();
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
