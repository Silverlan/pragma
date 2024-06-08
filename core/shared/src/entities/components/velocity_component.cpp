/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/util/util_ballistic.h"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

using namespace pragma;

constexpr float VELOCITY_EPSILON_DELTA_FOR_SNAPSHOT = 0.05f;

void VelocityComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = VelocityComponent;

	using TVelocity = Vector3;
	registerMember(create_component_member_info<T, TVelocity, static_cast<void (T::*)(const TVelocity &)>(&T::SetVelocity), static_cast<const TVelocity &(T::*)() const>(&T::GetVelocity)>("velocity", TVelocity {}));
}
VelocityComponent::VelocityComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_velocity(util::Vector3Property::Create()), m_angVelocity(util::Vector3Property::Create()) {}
void VelocityComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("transform");
}
void VelocityComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
const util::PVector3Property &VelocityComponent::GetVelocityProperty() const { return m_velocity; }
const util::PVector3Property &VelocityComponent::GetAngularVelocityProperty() const { return m_angVelocity; }

util::EventReply VelocityComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == DamageableComponent::EVENT_ON_TAKE_DAMAGE) {
		auto &force = static_cast<CEOnTakeDamage &>(evData).damageInfo.GetForce();
		AddVelocity(force);
	}
	else if(eventId == BaseTransformComponent::EVENT_ON_TELEPORT) {
		auto &te = static_cast<CETeleport &>(evData);
		auto vel = GetVelocity();
		uvec::rotate(&vel, te.deltaPose.GetRotation());
		SetVelocity(vel);
	}
	return util::EventReply::Unhandled;
}

void VelocityComponent::SetVelocity(const Vector3 &vel)
{
	auto dt = uvec::distance_sqr(vel, *m_velocity);
	*m_velocity = vel;
	if(dt > VELOCITY_EPSILON_DELTA_FOR_SNAPSHOT)
		GetEntity().MarkForSnapshot(true);
}

void VelocityComponent::AddVelocity(const Vector3 &vel) { SetVelocity(GetVelocity() + vel); }

const Vector3 &VelocityComponent::GetVelocity() const { return *m_velocity; }
void VelocityComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["velocity"] = **m_velocity;
	udm["angularVelocity"] = **m_angVelocity;
}

void VelocityComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	Vector3 vel {};
	udm["velocity"](vel);
	SetVelocity(vel);

	Vector3 angVel {};
	udm["angularVelocity"](angVel);
	SetAngularVelocity(angVel);
}
void VelocityComponent::SetAngularVelocity(const Vector3 &vel)
{
	auto dt = uvec::distance_sqr(vel, *m_velocity);
	*m_angVelocity = vel;
	if(dt > VELOCITY_EPSILON_DELTA_FOR_SNAPSHOT)
		GetEntity().MarkForSnapshot(true);
}
void VelocityComponent::AddAngularVelocity(const Vector3 &vel) { SetAngularVelocity(GetAngularVelocity() + vel); }
const Vector3 &VelocityComponent::GetAngularVelocity() const { return *m_angVelocity; }
void VelocityComponent::SetLocalAngularVelocity(Vector3 vel)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr)
		uvec::rotate(&vel, pTrComponent->GetRotation());
	SetAngularVelocity(vel);
}
void VelocityComponent::AddLocalAngularVelocity(Vector3 vel)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr)
		uvec::rotate(&vel, pTrComponent->GetRotation());
	AddAngularVelocity(vel);
}
Vector3 VelocityComponent::GetLocalAngularVelocity() const
{
	auto vel = GetAngularVelocity();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr) {
		auto rot = pTrComponent->GetRotation();
		uquat::inverse(rot);
		uvec::rotate(&vel, rot);
	}
	return vel;
}
void VelocityComponent::SetLocalVelocity(Vector3 vel)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr)
		uvec::rotate(&vel, pTrComponent->GetRotation());
	SetVelocity(vel);
}
void VelocityComponent::AddLocalVelocity(Vector3 vel)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr)
		uvec::rotate(&vel, pTrComponent->GetRotation());
	AddVelocity(vel);
}
Vector3 VelocityComponent::GetLocalVelocity() const
{
	auto vel = GetVelocity();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr) {
		auto rot = pTrComponent->GetRotation();
		uquat::inverse(rot);
		uvec::rotate(&vel, rot);
	}
	return vel;
}

void VelocityComponent::SetRawVelocity(const Vector3 &vel) { *m_velocity = vel; }
void VelocityComponent::SetRawAngularVelocity(const Vector3 &vel) { *m_angVelocity = vel; }
