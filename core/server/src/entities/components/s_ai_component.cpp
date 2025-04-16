/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_model_component.hpp"
#include "pragma/entities/components/s_animated_component.hpp"
#include "pragma/entities/components/s_health_component.hpp"
#include "pragma/entities/components/s_physics_component.hpp"
#include "pragma/ai/s_npcstate.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/ai/ai_squad.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/game/s_game.h"
#include "pragma/entities/player.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/model/model.h>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_io_component.hpp>
#include <pragma/entities/components/damageable_component.hpp>
#include <pragma/entities/components/movement_component.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/logging.hpp>

extern DLLSERVER SGame *s_game;

using namespace pragma;

namespace pragma {
	using ::operator<<;
};

decltype(SAIComponent::s_npcs) SAIComponent::s_npcs {};
decltype(SAIComponent::s_factionManager) SAIComponent::s_factionManager {};
FactionManager &SAIComponent::GetFactionManager() { return s_factionManager; }
const std::vector<pragma::SAIComponent *> &SAIComponent::GetAll() { return s_npcs; }
unsigned int SAIComponent::GetNPCCount() { return CUInt32(s_npcs.size()); }

ComponentEventId SAIComponent::EVENT_SELECT_SCHEDULE = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_SELECT_CONTROLLER_SCHEDULE = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_SCHEDULE_COMPLETE = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_PRIMARY_TARGET_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_PATH_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_NPC_STATE_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_TARGET_VISIBILITY_LOST = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_TARGET_VISIBILITY_REACQUIRED = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_MEMORY_GAINED = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_MEMORY_LOST = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_TARGET_ACQUIRED = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_SUSPICIOUS_SOUND_HEARED = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_CONTROLLER_ACTION_INPUT = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_START_CONTROL = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_END_CONTROL = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_PATH_NODE_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_LOOK_TARGET_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId SAIComponent::EVENT_ON_SCHEDULE_STARTED = INVALID_COMPONENT_ID;
void SAIComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseAIComponent::RegisterEvents(componentManager, registerEvent);
	EVENT_SELECT_SCHEDULE = registerEvent("SELECT_SCHEDULE", ComponentEventInfo::Type::Broadcast);
	EVENT_SELECT_CONTROLLER_SCHEDULE = registerEvent("SELECT_CONTROLLER_SCHEDULE", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_SCHEDULE_COMPLETE = registerEvent("ON_SCHEDULE_COMPLETE", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_PRIMARY_TARGET_CHANGED = registerEvent("ON_PRIMARY_TARGET_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_PATH_CHANGED = registerEvent("ON_PATH_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_NPC_STATE_CHANGED = registerEvent("ON_NPC_STATE_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_TARGET_VISIBILITY_LOST = registerEvent("ON_TARGET_VISIBILITY_LOST", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_TARGET_VISIBILITY_REACQUIRED = registerEvent("ON_TARGET_VISIBILITY_REACQUIRED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_MEMORY_GAINED = registerEvent("ON_MEMORY_GAINED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_MEMORY_LOST = registerEvent("ON_MEMORY_LOST", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_TARGET_ACQUIRED = registerEvent("ON_TARGET_ACQUIRED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_SUSPICIOUS_SOUND_HEARED = registerEvent("ON_SUSPICIOUS_SOUND_HEARED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_CONTROLLER_ACTION_INPUT = registerEvent("ON_CONTROLLER_ACTION_INPUT", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_START_CONTROL = registerEvent("ON_START_CONTROL", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_END_CONTROL = registerEvent("ON_END_CONTROL", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_PATH_NODE_CHANGED = registerEvent("ON_PATH_NODE_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_LOOK_TARGET_CHANGED = registerEvent("ON_LOOK_TARGET_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_SCHEDULE_STARTED = registerEvent("ON_SCHEDULE_STARTED", ComponentEventInfo::Type::Broadcast);
}

SAIComponent::SAIComponent(BaseEntity &ent) : BaseAIComponent(ent), m_npcState(NPCSTATE::NONE)
{
	SetMaxViewAngle(70.f);
	static_cast<SBaseEntity &>(ent).SetShared(true);
	s_npcs.push_back(this);
}

SAIComponent::~SAIComponent()
{
	ClearRelationships();
	auto it = std::find(s_npcs.begin(), s_npcs.end(), this);
	if(it != s_npcs.end())
		s_npcs.erase(it);
}

void SAIComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SAIComponent::OnRemove()
{
	BaseAIComponent::OnRemove();
	if(m_schedule != nullptr)
		m_schedule->Cancel();
	EndControl();
	SetSquad("");
}

void SAIComponent::OnLookTargetChanged()
{
	BaseAIComponent::OnLookTargetChanged();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		p->Write<BaseAIComponent::LookTargetType>(m_neckInfo.lookTargetType);
		switch(m_neckInfo.lookTargetType) {
		case BaseAIComponent::LookTargetType::Position:
			p->Write<Vector3>(m_neckInfo.lookTarget);
			break;
		case BaseAIComponent::LookTargetType::Entity:
			nwm::write_entity(p, m_neckInfo.hEntityLookTarget);
			break;
		}
		ent.SendNetEvent(m_netEvSetLookTarget, p, pragma::networking::Protocol::SlowReliable);
	}
	BroadcastEvent(EVENT_ON_LOOK_TARGET_CHANGED);
}

bool SAIComponent::IsAIEnabled() const
{
	auto charComponent = GetEntity().GetCharacterComponent();
	return (m_bAiEnabled && (charComponent.expired() || charComponent->IsAlive())) ? true : false;
}
void SAIComponent::SetAIEnabled(bool b)
{
	m_bAiEnabled = b;
	if(b == false) {
		CancelSchedule();
		ClearMemory();
	}
}
void SAIComponent::EnableAI() { SetAIEnabled(true); }
void SAIComponent::DisableAI() { SetAIEnabled(false); }

std::shared_ptr<ai::Schedule> SAIComponent::GetCurrentSchedule()
{
	if(m_schedule == nullptr)
		return {};
	return m_schedule;
}

void SAIComponent::Initialize()
{
	BaseAIComponent::Initialize();

	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(OnInput(inputData.input, inputData.activator, inputData.caller, inputData.data))
			return util::EventReply::Handled;
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(DamageableComponent::EVENT_ON_TAKE_DAMAGE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnTakeDamage(static_cast<CEOnTakeDamage &>(evData.get()).damageInfo); });
	BindEventUnhandled(SHealthComponent::EVENT_ON_TAKEN_DAMAGE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &evDataDmg = static_cast<CEOnTakenDamage &>(evData.get());
		OnTakenDamage(evDataDmg.damageInfo, evDataDmg.oldHealth, evDataDmg.newHealth);
	});
	BindEventUnhandled(SPhysicsComponent::EVENT_ON_PRE_PHYSICS_SIMULATE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnPrePhysicsSimulate(); });
	BindEventUnhandled(SModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		pragma::SAIComponent::AIAnimationInfo info {};
		info.SetPlayAsSchedule(false);
		PlayActivity(Activity::Idle, info);
	});
	BindEventUnhandled(SAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(m_schedule == nullptr && IsMoving() == false) {
			pragma::SAIComponent::AIAnimationInfo info {};
			info.SetPlayAsSchedule(false);
			PlayActivity(Activity::Idle, info);
		}
		/*auto *task = GetCurrentTask();
		if(task == NULL)
			return;
		switch(task->GetID())
		{
		case AI_TASK_PLAY_ACTIVITY:
		case AI_TASK_PLAY_ANIMATION:
			{
				CompleteTask();
				break;
			}
		};*/ // TODO
	});
	BindEvent(BaseAnimatedComponent::EVENT_ON_PLAY_ANIMATION, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		if(m_bSkipHandling == true)
			return util::EventReply::Unhandled;
		AIAnimationInfo info {};
		auto &animData = static_cast<CEOnPlayAnimation &>(evData.get());
		PlayAnimation(animData.animation, info);
		return util::EventReply::Handled;
	});

	BindEventUnhandled(SAnimatedComponent::EVENT_MAINTAIN_ANIMATION_MOVEMENT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { MaintainAnimationMovement(static_cast<CEMaintainAnimationMovement &>(evData.get()).displacement); });
}

void SAIComponent::UpdateMovementProperties(MovementComponent &movementC)
{
	BaseAIComponent::UpdateMovementProperties(movementC);
	if(m_animMoveInfo.moving == false && m_moveInfo.moving == false)
		movementC.SetSpeed({0.f, 0.f});
}

void SAIComponent::OnEntitySpawn()
{
	BaseAIComponent::OnEntitySpawn();
	auto &ent = GetEntity();

	pragma::SAIComponent::AIAnimationInfo info {};
	info.SetPlayAsSchedule(false);
	PlayActivity(Activity::Idle, info);

	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->DropToFloor();
}

bool SAIComponent::OnInput(std::string input, BaseEntity *activator, BaseEntity *caller, const std::string &data)
{
#if DEBUG_AI_MOVEMENT == 1
	if(ustring::compare<std::string>(input, "dbg_move", false)) {
		auto pTrComponentActivator = (activator != nullptr) ? activator->GetTransformComponent() : nullptr;
		if(pTrComponentActivator) {
			pragma::SAIComponent::AIAnimationInfo info {};
			info.SetPlayAsSchedule(false);
			PlayActivity(m_moveInfo.moveActivity, info);

			auto &ent = GetEntity();
			BaseAIComponent::MoveInfo mvInfo {};
			if(data == "1") {
				auto pTrComponent = ent.GetTransformComponent();
				if(pTrComponent != nullptr)
					mvInfo.faceTarget = pTrComponent->GetPosition() + pTrComponent->GetForward() * 10'000.f;
			}
			auto r = MoveTo(pTrComponentActivator->GetPosition(), mvInfo);
			if(r == BaseAIComponent::MoveResult::TargetUnreachable) {
				spdlog::info("Unable to move on path; Attempting to move by LoS...");
				mvInfo.moveOnPath = false;
				r = MoveTo(pTrComponentActivator->GetPosition(), mvInfo);
			}
			spdlog::info("MoveTo result: {}", BaseAIComponent::MoveResultToString(r));
		}
	}
#endif
	else
		return false;
	return true;
}
void SAIComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}
void SAIComponent::SendSnapshotData(NetPacket &packet, pragma::BasePlayerComponent &pl)
{
	auto snapshotFlags = SnapshotFlags::None;
	if(m_moveInfo.moving == false)
		packet->Write<SnapshotFlags>(snapshotFlags);
	else {
		snapshotFlags |= SnapshotFlags::Moving;
		if(m_moveInfo.moveSpeed != nullptr)
			snapshotFlags |= SnapshotFlags::MoveSpeed;
		if(m_moveInfo.turnSpeed != nullptr)
			snapshotFlags |= SnapshotFlags::TurnSpeed;
		if(m_moveInfo.faceTarget != nullptr)
			snapshotFlags |= SnapshotFlags::FaceTarget;
		packet->Write<SnapshotFlags>(snapshotFlags);

		packet->Write<Activity>(m_moveInfo.moveActivity);
		packet->Write<Vector3>(m_moveInfo.moveDir);
		packet->Write<Vector3>(m_moveInfo.moveTarget);
		if((snapshotFlags & SnapshotFlags::MoveSpeed) != SnapshotFlags::None)
			packet->Write<float>(*m_moveInfo.moveSpeed);
		if((snapshotFlags & SnapshotFlags::TurnSpeed) != SnapshotFlags::None)
			packet->Write<float>(*m_moveInfo.turnSpeed);
		if((snapshotFlags & SnapshotFlags::FaceTarget) != SnapshotFlags::None)
			packet->Write<Vector3>(*m_moveInfo.faceTarget);
	}
}

void SAIComponent::SetSquad(std::string squadName)
{
	if(squadName.empty()) {
		if(m_squad == NULL)
			return;
		m_squad->Leave(&GetEntity());
		m_squad = NULL;
		return;
	}
	StringToLower(squadName);
	auto &squads = AISquad::GetAll();
	auto it = std::find_if(squads.begin(), squads.end(), [&squadName](const std::shared_ptr<AISquad> &squad) { return (squad->name == squadName) ? true : false; });
	if(it != squads.end()) {
		auto &squad = *it;
		m_squad = squad;
		m_squad->Join(&GetEntity());
		return;
	}
	m_squad = AISquad::Create(squadName);
	m_squad->Join(&GetEntity());
}
const std::shared_ptr<AISquad> &SAIComponent::GetSquad() const { return m_squad; }
std::string SAIComponent::GetSquadName()
{
	if(m_squad == nullptr)
		return "";
	return m_squad->name;
}

void SAIComponent::StartSchedule(std::shared_ptr<ai::Schedule> &sched)
{
	CancelSchedule();
	m_schedule = sched;
	auto r = m_schedule->Start(*this);
	OnScheduleStarted(m_schedule, r);
	if(r == ai::BehaviorNode::Result::Failed) {
		m_schedule->Cancel();
		OnScheduleComplete(m_schedule, r);
		m_schedule = nullptr;
	}
}

void SAIComponent::OnTick(double tDelta)
{
	auto &ent = GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.valid() && pTrComponent != nullptr)
		charComponent->SetViewOrientation(pTrComponent->GetRotation());
	BaseAIComponent::OnTick(tDelta); // Has to be called AFTER Entity::Think, to make sure animations are updated (Required for animation movement)
	if(IsAIEnabled() == false) {
		auto *controller = GetController();
		if(controller != nullptr) {
			if(m_schedule != nullptr)
				RunSchedule();
			if(m_schedule == nullptr)
				SelectControllerSchedule();
		}
		return;
	}
	UpdateMemory();
	if(m_schedule != nullptr)
		RunSchedule();
	auto &t = s_game->CurTime();
	if(t >= m_tNextEnemyCheck) {
		SelectEnemies();
		auto state = GetNPCState();
		if(state == NPCSTATE::ALERT || state == NPCSTATE::COMBAT)
			m_tNextEnemyCheck = CFloat(t + AI_NEXT_ENEMY_CHECK_ALERT);
		else
			m_tNextEnemyCheck = CFloat(t + AI_NEXT_ENEMY_CHECK_IDLE);
	}
	if(m_schedule == nullptr)
		SelectSchedule();
}

void SAIComponent::SelectSchedule() { BroadcastEvent(EVENT_SELECT_SCHEDULE); }

void SAIComponent::OnScheduleStarted(const std::shared_ptr<ai::Schedule> &schedule, ai::BehaviorNode::Result result)
{
	CEOnScheduleStateChanged evData {schedule, result};
	BroadcastEvent(EVENT_ON_SCHEDULE_STARTED, evData);
}

void SAIComponent::OnScheduleComplete(const std::shared_ptr<ai::Schedule> &schedule, ai::BehaviorNode::Result result)
{
	CEOnScheduleStateChanged evData {schedule, result};
	BroadcastEvent(EVENT_ON_SCHEDULE_COMPLETE, evData);
}

void SAIComponent::CancelSchedule()
{
	if(m_schedule == nullptr)
		return;
	m_schedule->Cancel();
	OnScheduleComplete(m_schedule, ai::BehaviorNode::Result::Failed);
	m_schedule = nullptr;
	StopMoving();
}

bool SAIComponent::TriggerScheduleInterrupt(uint32_t interruptFlags)
{
	if(m_schedule == nullptr)
		return true;
	if((m_schedule->GetInterruptFlags() & interruptFlags) != 0) {
		CancelSchedule();
		return true;
	}
	return false;
}

bool SAIComponent::IsEnemy(BaseEntity *ent) const
{
	auto disp = const_cast<SAIComponent *>(this)->GetDisposition(ent);
	return (disp == DISPOSITION::LIKE || disp == DISPOSITION::NEUTRAL) ? false : true;
}

void SAIComponent::RunSchedule()
{
	if(m_schedule == nullptr)
		return;
	auto r = m_schedule->Think(*this);
	if(r != ai::BehaviorNode::Result::Pending) // Schedule complete (Failed or succeeded)
	{
		m_schedule->Cancel();
		OnScheduleComplete(m_schedule, r);
		m_schedule = nullptr;
	}
}

bool SAIComponent::TurnStep(const Vector3 &target, float &turnAngle, const float *turnSpeed)
{
	auto r = BaseAIComponent::TurnStep(target, turnAngle, turnSpeed);
	if(IsAnimationLocked() == true)
		return r;
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	auto act = animComponent.valid() ? animComponent->GetActivity() : Activity::Invalid;

	if(r == false && IsMoving() == false && (act == Activity::Idle || act == Activity::Invalid)) {
		if(turnAngle < 0.f) {
			pragma::SAIComponent::AIAnimationInfo info {};
			info.SetPlayAsSchedule(false);
			if(PlayActivity(Activity::TurnLeft, info) == false) {
				PlayActivity(Activity::Idle, info);
				if(animComponent.valid())
					animComponent->PlayLayeredActivity(0, Activity::GestureTurnLeft);
			}
		}
		else {
			pragma::SAIComponent::AIAnimationInfo info {};
			info.SetPlayAsSchedule(false);
			if(PlayActivity(Activity::TurnRight, info) == false) {
				PlayActivity(Activity::Idle, info);
				if(animComponent.valid())
					animComponent->PlayLayeredActivity(0, Activity::GestureTurnRight);
			}
		}
	}
	else if(act == Activity::TurnLeft || act == Activity::TurnRight) {
		pragma::SAIComponent::AIAnimationInfo info {};
		info.SetPlayAsSchedule(false);
		PlayActivity(Activity::Idle, info);
	}
	//if(r == true)
	//	CompleteTask();
	return r;
}

NPCSTATE SAIComponent::GetNPCState() const { return m_npcState; }
void SAIComponent::SetNPCState(NPCSTATE state)
{
	auto oldState = m_npcState;
	m_npcState = state;
	OnNPCStateChanged(oldState, state);
}

void SAIComponent::OnTargetAcquired(BaseEntity *ent, float dist, bool isFirstNewTarget)
{
	CEOnTargetAcquired evData {ent, dist, isFirstNewTarget};
	BroadcastEvent(EVENT_ON_TARGET_ACQUIRED, evData);
}

void SAIComponent::SelectEnemies()
{
	auto numPrevTargets = GetMemoryFragmentCount();
	std::vector<TargetInfo> newTargets;
	Listen(newTargets);
	for(unsigned int i = 0; i < s_npcs.size(); i++) {
		SAIComponent *npc = s_npcs[i];
		auto &ent = npc->GetEntity();
		if(npc != this) {
			auto *charComponent = static_cast<pragma::SCharacterComponent *>(ent.GetCharacterComponent().get());
			if(charComponent == nullptr || (charComponent->IsAlive() == true && charComponent->GetNoTarget() == false)) {
				auto disp = GetDisposition(&ent);
				float dist;
				if(disp == DISPOSITION::HATE && !IsInMemory(&ent) && IsInViewCone(&ent, &dist)) {
					if(Memorize(&ent, ai::Memory::MemoryType::Visual) != nullptr)
						newTargets.push_back({&ent, dist});
				}
			}
		}
	}
	auto &players = SPlayerComponent::GetAll();
	for(auto *pl : players) {
		auto &ent = pl->GetEntity();
		auto *charComponent = static_cast<pragma::SCharacterComponent *>(ent.GetCharacterComponent().get());
		if(charComponent != nullptr && charComponent->IsAlive() == false)
			continue;
		auto disp = GetDisposition(&ent);
		float dist;
		if(disp == DISPOSITION::HATE && charComponent->GetNoTarget() == false && !IsInMemory(&ent) && IsInViewCone(&ent, &dist)) {
			if(Memorize(&ent, ai::Memory::MemoryType::Visual) != nullptr)
				newTargets.push_back({&ent, dist});
		}
	}
	SelectPrimaryTarget();
	auto bFirst = (numPrevTargets == 0) ? true : false;
	for(auto &tgt : newTargets) {
		OnTargetAcquired(tgt.ent, tgt.dist, bFirst);
		bFirst = false;
	}
}

void SAIComponent::OnNPCStateChanged(NPCSTATE oldState, NPCSTATE newState)
{
	CEOnNPCStateChanged evData {oldState, newState};
	BroadcastEvent(EVENT_ON_NPC_STATE_CHANGED, evData);
}

bool SAIComponent::IsObstruction(const BaseEntity &ent) const
{
	if(IsEnemy(&const_cast<BaseEntity &>(ent)) == true) // Don't count enemies as movement obstructions (We wouldn't want to move around them...)
		return false;
	return BaseAIComponent::IsObstruction(ent);
}

void SAIComponent::OnEntityComponentAdded(BaseEntityComponent &component) { BaseAIComponent::OnEntityComponentAdded(component); }

util::EventReply SAIComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseAIComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseCharacterComponent::EVENT_ON_KILLED)
		OnKilled(static_cast<const CEOnCharacterKilled &>(evData).damageInfo);
	return util::EventReply::Unhandled;
}

bool SAIComponent::HasCharacterNoTargetEnabled(const BaseEntity &ent) const
{
	auto *charComponent = static_cast<pragma::SCharacterComponent *>(ent.GetCharacterComponent().get());
	return (charComponent != nullptr) ? charComponent->GetNoTarget() : false;
}

//////////////////

CEMemoryData::CEMemoryData(const ai::Memory::Fragment *memoryFragment) : memoryFragment {memoryFragment} {}
void CEMemoryData::PushArguments(lua_State *l)
{
	if(memoryFragment != nullptr)
		Lua::Push<ai::Memory::Fragment *>(l, const_cast<ai::Memory::Fragment *>(memoryFragment));
	else
		Lua::PushNil(l);
}

//////////////////

CEOnNPCStateChanged::CEOnNPCStateChanged(NPCSTATE oldState, NPCSTATE newState) : oldState {oldState}, newState {newState} {}
void CEOnNPCStateChanged::PushArguments(lua_State *l)
{
	Lua::PushInt(l, umath::to_integral(oldState));
	Lua::PushInt(l, umath::to_integral(newState));
}

//////////////////

CEOnTargetAcquired::CEOnTargetAcquired(BaseEntity *entity, float distance, bool isFirstNewTarget) : entity {entity}, distance {distance}, isFirstNewTarget {isFirstNewTarget} {}
void CEOnTargetAcquired::PushArguments(lua_State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
	Lua::PushNumber(l, distance);
	Lua::PushBool(l, isFirstNewTarget);
}

//////////////////

CEOnControllerActionInput::CEOnControllerActionInput(Action action, bool pressed) : action {action}, pressed {pressed} {}
void CEOnControllerActionInput::PushArguments(lua_State *l)
{
	Lua::PushInt(l, umath::to_integral(action));
	Lua::PushBool(l, pressed);
}

//////////////////

CEOnSuspiciousSoundHeared::CEOnSuspiciousSoundHeared(const std::shared_ptr<ALSound> &sound) : sound {sound} {}
void CEOnSuspiciousSoundHeared::PushArguments(lua_State *l) { Lua::Push<std::shared_ptr<ALSound>>(l, sound); }

//////////////////

CEOnStartControl::CEOnStartControl(pragma::SPlayerComponent &player) : player {player} {}
void CEOnStartControl::PushArguments(lua_State *l) { player.PushLuaObject(l); }

//////////////////

CEOnPathNodeChanged::CEOnPathNodeChanged(uint32_t nodeIndex) : nodeIndex {nodeIndex} {}
void CEOnPathNodeChanged::PushArguments(lua_State *l) { Lua::PushInt(l, nodeIndex); }

//////////////////

CEOnScheduleStateChanged::CEOnScheduleStateChanged(const std::shared_ptr<ai::Schedule> &schedule, ai::BehaviorNode::Result result) : schedule {schedule}, result {result} {}
void CEOnScheduleStateChanged::PushArguments(lua_State *l)
{
	Lua::Push<std::shared_ptr<ai::Schedule>>(l, schedule);
	Lua::PushInt(l, umath::to_integral(result));
}
