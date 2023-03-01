/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_ai_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

std::vector<CAIComponent *> CAIComponent::s_npcs;
const std::vector<CAIComponent *> &CAIComponent::GetAll() { return s_npcs; }
unsigned int CAIComponent::GetNPCCount() { return CUInt32(s_npcs.size()); }

CAIComponent::CAIComponent(BaseEntity &ent) : BaseAIComponent(ent) { s_npcs.push_back(this); }

CAIComponent::~CAIComponent()
{
	for(int i = 0; i < s_npcs.size(); i++) {
		if(s_npcs[i] == this) {
			s_npcs.erase(s_npcs.begin() + i);
			break;
		}
	}
}
void CAIComponent::ReceiveSnapshotData(NetPacket &packet)
{
	auto snapshotFlags = packet->Read<SnapshotFlags>();
	if((snapshotFlags & SnapshotFlags::Moving) == SnapshotFlags::None) {
		if(m_moveInfo.moving == true) {
			m_moveInfo.moving = false;
			m_moveInfo.moveSpeed = nullptr;
			m_moveInfo.turnSpeed = nullptr;
			m_moveInfo.faceTarget = nullptr;
		}
	}
	else {
		m_moveInfo.moving = true;
		m_moveInfo.moveActivity = packet->Read<Activity>();
		m_moveInfo.moveDir = packet->Read<Vector3>();
		m_moveInfo.moveTarget = packet->Read<Vector3>();

		if((snapshotFlags & SnapshotFlags::MoveSpeed) != SnapshotFlags::None) {
			auto moveSpeed = packet->Read<float>();
			if(m_moveInfo.moveSpeed == nullptr)
				m_moveInfo.moveSpeed = std::make_unique<float>(moveSpeed);
			else
				*m_moveInfo.moveSpeed = moveSpeed;
		}
		else
			m_moveInfo.moveSpeed = nullptr;
		if((snapshotFlags & SnapshotFlags::TurnSpeed) != SnapshotFlags::None) {
			auto turnSpeed = packet->Read<float>();
			if(m_moveInfo.turnSpeed == nullptr)
				m_moveInfo.turnSpeed = std::make_unique<float>(turnSpeed);
			else
				*m_moveInfo.turnSpeed = turnSpeed;
		}
		else
			m_moveInfo.moveSpeed = nullptr;
		if((snapshotFlags & SnapshotFlags::FaceTarget) != SnapshotFlags::None) {
			auto faceTarget = packet->Read<Vector3>();
			if(m_moveInfo.faceTarget == nullptr)
				m_moveInfo.faceTarget = std::make_unique<Vector3>(faceTarget);
			else
				*m_moveInfo.faceTarget = faceTarget;
		}
		else
			m_moveInfo.faceTarget = nullptr;
	}
}
void CAIComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<CAIComponent>(l); }

void CAIComponent::ReceiveData(NetPacket &packet)
{
	// Note: Change return value of ShouldTransmitNetData if data should be received
}

bool CAIComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetLookTarget) {
		auto type = packet->Read<BaseAIComponent::LookTargetType>();
		switch(type) {
		case BaseAIComponent::LookTargetType::Position:
			{
				auto v = packet->Read<Vector3>();
				SetLookTarget(v);
				break;
			}
		case BaseAIComponent::LookTargetType::Entity:
			{
				auto *ent = nwm::read_entity(packet);
				if(ent == nullptr)
					ClearLookTarget();
				else
					SetLookTarget(*ent);
				break;
			}
		default:
			ClearLookTarget();
		}
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

Vector3 CAIComponent::OnCalcMovementDirection(const Vector3 &, const Vector3 &) const
{
	auto pVelComponent = GetEntity().GetComponent<pragma::VelocityComponent>();
	if(pVelComponent.expired())
		return {};
	auto vel = pVelComponent->GetVelocity(); // Client doesn't know the NPCs actual movement direction; Just assume it's the same as its velocity
	auto l = uvec::length(vel);
	if(l < 1.f)
		return {};
	return vel / l;
}

void CAIComponent::Initialize() { BaseAIComponent::Initialize(); }

void CAIComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseAIComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(pragma::CCharacterComponent)) {
		FlagCallbackForRemoval(static_cast<pragma::CCharacterComponent *>(&component)
		                         ->BindEvent(CCharacterComponent::EVENT_CALC_MOVEMENT_DIRECTION,
		                           [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			                           auto &dirData = static_cast<CECalcMovementDirection &>(evData.get());
			                           dirData.direction = OnCalcMovementDirection(dirData.forward, dirData.right);
			                           return util::EventReply::Handled;
		                           }),
		  CallbackType::Component, &component);
	}
}

util::EventReply CAIComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseAIComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	return util::EventReply::Unhandled;
}
