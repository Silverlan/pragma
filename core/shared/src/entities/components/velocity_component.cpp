/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/util/util_ballistic.h"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;

constexpr uint32_t VELOCITY_EPSILON_DELTA_FOR_SNAPSHOT = 0.05f;


VelocityComponent::VelocityComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),
	m_velocity(util::Vector3Property::Create()),
	m_angVelocity(util::Vector3Property::Create())
{}
void VelocityComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("transform");
}
luabind::object VelocityComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<VelocityComponentHandleWrapper>(l);}
const util::PVector3Property &VelocityComponent::GetVelocityProperty() const {return m_velocity;}
const util::PVector3Property &VelocityComponent::GetAngularVelocityProperty() const {return m_angVelocity;}

util::EventReply VelocityComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == DamageableComponent::EVENT_ON_TAKE_DAMAGE)
	{
		auto &force = static_cast<CEOnTakeDamage&>(evData).damageInfo.GetForce();
		AddVelocity(force);
	}
	return util::EventReply::Unhandled;
}

void VelocityComponent::SetVelocity(const Vector3 &vel)
{
	auto dt = uvec::distance_sqr(vel,*m_velocity);
	*m_velocity = vel;
	if(dt > VELOCITY_EPSILON_DELTA_FOR_SNAPSHOT)
		GetEntity().MarkForSnapshot(true);
}

void VelocityComponent::AddVelocity(const Vector3 &vel) {SetVelocity(GetVelocity() +vel);}

const Vector3 &VelocityComponent::GetVelocity() const {return *m_velocity;}
void VelocityComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);
	ds->Write<Vector3>(*m_velocity);
	ds->Write<Vector3>(*m_angVelocity);
}

void VelocityComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);
	auto vel = ds->Read<Vector3>();
	SetVelocity(vel);

	auto angVel = ds->Read<Vector3>();
	SetAngularVelocity(angVel);
}
void VelocityComponent::SetAngularVelocity(const Vector3 &vel)
{
	auto dt = uvec::distance_sqr(vel,*m_velocity);
	*m_angVelocity = vel;
	if(dt > VELOCITY_EPSILON_DELTA_FOR_SNAPSHOT)
		GetEntity().MarkForSnapshot(true);
}
void VelocityComponent::AddAngularVelocity(const Vector3 &vel) {SetAngularVelocity(GetAngularVelocity() +vel);}
const Vector3 &VelocityComponent::GetAngularVelocity() const {return *m_angVelocity;}
void VelocityComponent::SetLocalAngularVelocity(Vector3 vel)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
		uvec::rotate(&vel,pTrComponent->GetRotation());
	SetAngularVelocity(vel);
}
void VelocityComponent::AddLocalAngularVelocity(Vector3 vel)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
		uvec::rotate(&vel,pTrComponent->GetRotation());
	AddAngularVelocity(vel);
}
Vector3 VelocityComponent::GetLocalAngularVelocity() const
{
	auto vel = GetAngularVelocity();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
	{
		auto rot = pTrComponent->GetRotation();
		uquat::inverse(rot);
		uvec::rotate(&vel,rot);
	}
	return vel;
}
void VelocityComponent::SetLocalVelocity(Vector3 vel)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
		uvec::rotate(&vel,pTrComponent->GetRotation());
	SetVelocity(vel);
}
void VelocityComponent::AddLocalVelocity(Vector3 vel)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
		uvec::rotate(&vel,pTrComponent->GetRotation());
	AddVelocity(vel);
}
Vector3 VelocityComponent::GetLocalVelocity() const
{
	auto vel = GetVelocity();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
	{
		auto rot = pTrComponent->GetRotation();
		uquat::inverse(rot);
		uvec::rotate(&vel,rot);
	}
	return vel;
}

void VelocityComponent::SetRawVelocity(const Vector3 &vel) {*m_velocity = vel;}
void VelocityComponent::SetRawAngularVelocity(const Vector3 &vel) {*m_angVelocity = vel;}

