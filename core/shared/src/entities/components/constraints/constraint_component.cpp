/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/components/constraints/constraint_manager_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/component_member_flags.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/logging.hpp"

using namespace pragma;

ComponentEventId ConstraintComponent::EVENT_APPLY_CONSTRAINT = pragma::INVALID_COMPONENT_ID;
ComponentEventId ConstraintComponent::EVENT_ON_DRIVER_CHANGED = pragma::INVALID_COMPONENT_ID;
ComponentEventId ConstraintComponent::EVENT_ON_DRIVEN_OBJECT_CHANGED = pragma::INVALID_COMPONENT_ID;
ComponentEventId ConstraintComponent::EVENT_ON_ORDER_INDEX_CHANGED = pragma::INVALID_COMPONENT_ID;
ComponentEventId ConstraintComponent::EVENT_ON_PARTICIPANTS_FLAGGED_DIRTY = pragma::INVALID_COMPONENT_ID;
void ConstraintComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_APPLY_CONSTRAINT = registerEvent("APPLY_CONSTRAINT", ComponentEventInfo::Type::Explicit);
	EVENT_ON_DRIVER_CHANGED = registerEvent("ON_DRIVER_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_DRIVEN_OBJECT_CHANGED = registerEvent("ON_DRIVEN_OBJECT_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_ORDER_INDEX_CHANGED = registerEvent("ON_ORDER_INDEX_CHANGED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_PARTICIPANTS_FLAGGED_DIRTY = registerEvent("ON_PARTICIPANTS_FLAGGED_DIRTY", ComponentEventInfo::Type::Broadcast);
}
void ConstraintComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintComponent;

	{
		using TInfluence = float;
		auto memberInfo = create_component_member_info<T, TInfluence, static_cast<void (T::*)(TInfluence)>(&T::SetInfluence), static_cast<TInfluence (T::*)() const>(&T::GetInfluence)>("influence", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(1.f);
		memberInfo.SetStepSize(0.01f);
		registerMember(std::move(memberInfo));
	}

	{
		using TCoordinateSpace = CoordinateSpace;
		auto memberInfo = create_component_member_info<T, TCoordinateSpace, static_cast<void (T::*)(TCoordinateSpace)>(&T::SetDriverSpace), static_cast<TCoordinateSpace (T::*)() const>(&T::GetDriverSpace)>("driverSpace", TCoordinateSpace::World);
		registerMember(std::move(memberInfo));
	}

	{
		using TCoordinateSpace = CoordinateSpace;
		auto memberInfo = create_component_member_info<T, TCoordinateSpace, static_cast<void (T::*)(TCoordinateSpace)>(&T::SetDrivenObjectSpace), static_cast<TCoordinateSpace (T::*)() const>(&T::GetDrivenObjectSpace)>("drivenObjectSpace", TCoordinateSpace::World);
		registerMember(std::move(memberInfo));
	}

	{
		using TDrivenObject = pragma::EntityUComponentMemberRef;
		auto memberInfo = create_component_member_info<T, TDrivenObject, static_cast<void (T::*)(const TDrivenObject &)>(&T::SetDrivenObject), static_cast<const TDrivenObject &(T::*)() const>(&T::GetDrivenObject)>("drivenObject", TDrivenObject {});
		registerMember(std::move(memberInfo));
	}

	{
		using TDriver = pragma::EntityUComponentMemberRef;
		auto memberInfo = create_component_member_info<T, TDriver, static_cast<void (T::*)(const TDriver &)>(&T::SetDriver), static_cast<const TDriver &(T::*)() const>(&T::GetDriver)>("driver", TDriver {});
		registerMember(std::move(memberInfo));
	}

	{
		using TOrderIndex = int32_t;
		auto memberInfo = create_component_member_info<T, TOrderIndex, static_cast<void (T::*)(TOrderIndex)>(&T::SetOrderIndex), static_cast<TOrderIndex (T::*)() const>(&T::GetOrderIndex)>("orderIndex", TOrderIndex {0});
		memberInfo.SetMin(-100);
		memberInfo.SetMax(100);
		registerMember(std::move(memberInfo));
	}
}
ConstraintComponent::ConstraintComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintComponent::Initialize() { BaseEntityComponent::Initialize(); }
void ConstraintComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintComponent::ApplyConstraint() { InvokeEventCallbacks(EVENT_APPLY_CONSTRAINT); }

void ConstraintComponent::SetDriverEnabled(bool enabled) { m_hasDriver = enabled; }
bool ConstraintComponent::HasDriver() const { return m_hasDriver; }

void ConstraintComponent::SetConstraintParticipantsDirty()
{
	if(!m_constraintParticipants)
		return;
	m_constraintParticipants = {};
	InvokeEventCallbacks(EVENT_ON_PARTICIPANTS_FLAGGED_DIRTY);
}

const std::optional<ConstraintComponent::ConstraintParticipants> &ConstraintComponent::GetConstraintParticipants() const { return const_cast<ConstraintComponent *>(this)->UpdateConstraintParticipants(); }

const std::optional<ConstraintComponent::ConstraintParticipants> &ConstraintComponent::UpdateConstraintParticipants()
{
	if(m_constraintParticipants) {
		if(m_constraintParticipants->drivenObjectC.valid() && (!m_hasDriver || m_constraintParticipants->driverC.valid()))
			return m_constraintParticipants;
	}
	auto &game = *GetEntity().GetNetworkState()->GetGameState();

	auto &drivenObj = GetDrivenObject();
	auto *drivenObjC = drivenObj.GetComponent(game);
	drivenObj.UpdateMemberIndex(game);
	auto idxDrivenObject = drivenObj.GetMemberIndex();
	if(!drivenObjC || idxDrivenObject == pragma::INVALID_COMPONENT_MEMBER_INDEX) {
		if(m_constraintParticipants)
			m_constraintParticipants = {};
		return m_constraintParticipants;
	}

	ConstraintParticipants participants {};
	participants.drivenObjectC = const_cast<pragma::BaseEntityComponent *>(drivenObjC)->GetHandle();
	participants.drivenObjectPropIdx = idxDrivenObject;

	auto *entDrivenObj = drivenObj.GetEntity(GetGame());
	if(entDrivenObj) {
		auto panimaC = entDrivenObj->GetComponent<pragma::PanimaComponent>();
		if(panimaC.valid()) {
			auto *memberInfo = drivenObj.GetMemberInfo(GetGame());
			if(memberInfo) {
				// TODO: Should we undo the dirty flag after the constraint has been removed?
				panimaC->SetPropertyAlwaysDirty(memberInfo->GetName(), true);
			}
		}
	}

	if(!m_hasDriver) {
		m_constraintParticipants = std::move(participants);
		return m_constraintParticipants;
	}

	auto &driver = GetDriver();
	auto *driverC = driver.GetComponent(game);
	driver.UpdateMemberIndex(game);
	auto idxDriver = driver.GetMemberIndex();
	if(!driverC || idxDriver == pragma::INVALID_COMPONENT_MEMBER_INDEX) {
		if(m_constraintParticipants)
			m_constraintParticipants = {};
		return m_constraintParticipants;
	}

	participants.driverC = const_cast<pragma::BaseEntityComponent *>(driverC)->GetHandle();
	participants.driverPropIdx = idxDriver;
	m_constraintParticipants = std::move(participants);
	return m_constraintParticipants;
}

void ConstraintComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }
void ConstraintComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_curDrivenConstraintManager.valid())
		m_curDrivenConstraintManager->RemoveConstraint(*this);
}

void ConstraintComponent::SetInfluence(float influence) { m_influence = influence; }
float ConstraintComponent::GetInfluence() const { return m_influence; }

void ConstraintComponent::SetDriver(const pragma::EntityUComponentMemberRef &driver)
{
	SetConstraintParticipantsDirty();
	m_driver = driver;
	BroadcastEvent(EVENT_ON_DRIVER_CHANGED);
}
const pragma::EntityUComponentMemberRef &ConstraintComponent::GetDriver() const { return m_driver; }

void ConstraintComponent::OnTick(double tDelta)
{
	BaseEntityComponent::OnTick(tDelta);
	auto *ent = m_drivenObject.GetEntity(*GetEntity().GetNetworkState()->GetGameState());
	if(!ent)
		return;
	spdlog::debug("Constraint driven object '{}' for constraint '{}' has been spawned. Initializing constraint...", ent->ToString(), GetEntity().ToString());
	SetDrivenObject(m_drivenObject);
}

void ConstraintComponent::SetDrivenObject(const pragma::EntityUComponentMemberRef &drivenObject)
{
	SetConstraintParticipantsDirty();
	if(m_curDrivenConstraintManager.valid())
		m_curDrivenConstraintManager->RemoveConstraint(*this);
	m_drivenObject = drivenObject;
	m_registeredWithConstraintManager = false;
	auto *ent = drivenObject.GetEntity(*GetEntity().GetNetworkState()->GetGameState());
	if(!ent) {
		// Entity doesn't exist (yet?), so we have to wait
		// until it exists before we can register the constraint
		SetTickPolicy(pragma::TickPolicy::Always);
		m_curDrivenConstraintManager = pragma::ComponentHandle<pragma::ConstraintManagerComponent> {};
		auto uuid = drivenObject.GetUuid();
		spdlog::debug("Constraint driven object '{}' does not exist for constraint '{}'. Listening for future instancing...", uuid.has_value() ? util::uuid_to_string(*uuid) : "", GetEntity().ToString());
		BroadcastEvent(EVENT_ON_DRIVEN_OBJECT_CHANGED);
		return;
	}
	auto constraintManagerC = const_cast<BaseEntity *>(ent)->AddComponent<ConstraintManagerComponent>();
	assert(constraintManagerC.valid());
	constraintManagerC->AddConstraint(*this);
	m_curDrivenConstraintManager = constraintManagerC;
	m_registeredWithConstraintManager = true;
	SetTickPolicy(pragma::TickPolicy::Never);
	BroadcastEvent(EVENT_ON_DRIVEN_OBJECT_CHANGED);
}
const pragma::EntityUComponentMemberRef &ConstraintComponent::GetDrivenObject() const { return m_drivenObject; }

void ConstraintComponent::SetDriverSpace(CoordinateSpace space) { m_driverSpace = space; }
pragma::CoordinateSpace ConstraintComponent::GetDriverSpace() const { return m_driverSpace; }

void ConstraintComponent::SetDrivenObjectSpace(CoordinateSpace space) { m_drivenObjectSpace = space; }
pragma::CoordinateSpace ConstraintComponent::GetDrivenObjectSpace() const { return m_drivenObjectSpace; }

void ConstraintComponent::SetOrderIndex(int32_t idx)
{
	if(idx == m_orderIndex)
		return;
	auto oldOrderIndex = m_orderIndex;
	if(m_curDrivenConstraintManager.valid())
		m_curDrivenConstraintManager->ChangeOrder(*this, idx); // constraint manager will change m_orderIndex
	else
		m_orderIndex = idx;
	InvokeEventCallbacks(EVENT_ON_ORDER_INDEX_CHANGED, CEOnConstraintOrderIndexChanged {oldOrderIndex, m_orderIndex});
}
int32_t ConstraintComponent::GetOrderIndex() const { return m_orderIndex; }

////////////

CEOnConstraintOrderIndexChanged::CEOnConstraintOrderIndexChanged(int32_t oldOrderIndex, int32_t newOrderIndex) : oldOrderIndex {oldOrderIndex}, newOrderIndex {newOrderIndex} {}
void CEOnConstraintOrderIndexChanged::PushArguments(lua_State *l)
{
	Lua::PushInt(l, oldOrderIndex);
	Lua::PushInt(l, newOrderIndex);
}
