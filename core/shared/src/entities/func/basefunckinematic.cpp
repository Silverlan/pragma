// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/func/basefunckinematic.hpp"
#include "pragma/entities/components/basepointpathnode.h"
#include "pragma/physics/physicstypes.h"
#include <sharedutils/util.h>
#include <sharedutils/util_string.h>
#include "pragma/util/util_handled.hpp"
#include "pragma/audio/alsound_type.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_sound_emitter_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BaseFuncKinematicComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "first_node", false))
			m_kvFirstNode = kvData.value;
		else if(ustring::compare<std::string>(kvData.key, "move_speed", false))
			m_kvMoveSpeed = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "start_sound", false))
			m_kvStartSound = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "startforward", false))
			StartForward();
		else if(ustring::compare<std::string>(inputData.input, "startbackward", false))
			StartBackward();
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("physics");
	ent.AddComponent("model");
	ent.AddComponent("io");
	ent.AddComponent("name");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);
	m_netEvStartForward = SetupNetEvent("start_forward");
	m_netEvStartBackward = SetupNetEvent("start_backward");
}

void BaseFuncKinematicComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr) {
		pPhysComponent->InitializePhysics(PHYSICSTYPE::DYNAMIC);
		pPhysComponent->SetKinematic(true);
	}
	NetworkState *state = ent.GetNetworkState();
	if(m_kvFirstNode.empty() == false) {
		Game *game = state->GetGameState();
		EntityIterator entIt {*game};
		//entIt.AttachFilter<EntityIteratorFilterComponent>("path_node");
		entIt.AttachFilter<EntityIteratorFilterEntity>(m_kvFirstNode);
		auto it = entIt.begin();
		if(it != entIt.end()) {
			m_nextNode = (*it)->GetHandle();
			UpdateTickPolicy();
		}
	}
	if(!m_kvStartSound.empty()) {
		state->PrecacheSound(m_kvStartSound);
		m_startSound = nullptr;
		auto pSoundEmitterComponent = static_cast<pragma::BaseSoundEmitterComponent *>(ent.FindComponent("sound_emitter").get());
		if(pSoundEmitterComponent != nullptr)
			m_startSound = pSoundEmitterComponent->CreateSound(m_kvStartSound, ALSoundType::Effect);
	}
}

void BaseFuncKinematicComponent::OnTick(double tDelta)
{
	if(m_bMoving == false || m_nextNode.valid() == false)
		return;
	MoveToTarget(m_nextNode.get(), m_speed); // Shouldnt be in think (PhysicsUpdate)
}

void BaseFuncKinematicComponent::UpdateTickPolicy() { SetTickPolicy((m_bMoving && m_nextNode.valid()) ? TickPolicy::Always : TickPolicy::Never); }

void BaseFuncKinematicComponent::MoveToTarget(BaseEntity *node, float speed)
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || phys->IsStatic())
		return;
	auto pTrComponent = ent.GetTransformComponent();
	auto *kinematic = dynamic_cast<PhysObjKinematic *>(phys);
	auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	auto pTrComponentNode = node->GetTransformComponent();
	auto posTarget = pTrComponentNode ? pTrComponentNode->GetPosition() : Vector3 {};
	auto dir = posTarget - pos;
	uvec::normalize(&dir);
	float d = glm::distance(pos, posTarget);
	if(d > speed)
		posTarget = pos + dir * speed;
	// SetKinematicTarget is deprecated TODO: Fix code below
	//kinematic->SetKinematicTarget(posTarget);
	if(d <= speed) {
		m_nextNode = EntityHandle {};
		UpdateTickPolicy();
		auto *ptrPathNodeComponent = static_cast<pragma::BasePointPathNodeComponent *>(node->FindComponent("path_node").get());
		if(ptrPathNodeComponent != nullptr) {
			auto *nodeNext = ptrPathNodeComponent->GetNextNode();
			if(nodeNext == nullptr)
				return;
			auto &entNext = nodeNext->GetEntity();
			m_nextNode = entNext.GetHandle();
			UpdateTickPolicy();
			MoveToTarget(&entNext, speed - d);
		}
	}
}

void BaseFuncKinematicComponent::StartForward()
{
	m_bMoving = true;
	UpdateTickPolicy();
	m_speed = m_kvMoveSpeed;

	if(m_startSound != nullptr)
		m_startSound->Play();
}

void BaseFuncKinematicComponent::StartBackward() {}
