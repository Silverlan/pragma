/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/orientation_component.hpp"

using namespace pragma;

OrientationComponent::OrientationComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_upDirection(util::Vector3Property::Create({0.f, 1.f, 0.f})) {}
void OrientationComponent::Initialize() { BaseEntityComponent::Initialize(); }
void OrientationComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
const util::PVector3Property &OrientationComponent::GetUpDirectionProperty() const { return m_upDirection; }

const Vector3 &OrientationComponent::GetUpDirection() const { return *m_upDirection; }

void OrientationComponent::GetOrientationAxes(Vector3 *outForward, Vector3 *outRight, Vector3 *outUp) const
{
	if(outForward != nullptr)
		*outForward = m_axForward;
	if(outRight != nullptr)
		*outRight = m_axRight;
	if(outUp != nullptr)
		*outUp = **m_upDirection;
}
const Quat &OrientationComponent::GetOrientationAxesRotation() const { return m_axRot; }

void OrientationComponent::SetUpDirection(const Vector3 &direction)
{
	auto ndirection = direction;
	uvec::normalize(&ndirection);
	*m_upDirection = ndirection;

	//m_axRot = uquat::create(uvec::FORWARD,uvec::RIGHT,m_upDirection);//uvec::UP);

	m_axRot = uvec::get_rotation(Vector3(0.f, 1.f, 0.f), *m_upDirection);
	uquat::normalize(m_axRot);
	m_axForward = Vector3(0.f, 0.f, 1.f) * m_axRot;
	m_axRight = Vector3(-1.f, 0.f, 0.f) * m_axRot;

	//auto &ent = GetEntity();
	//PhysObj *phys = ent.GetPhysicsObject();
	//if(phys == NULL || !phys->IsController())
	//	return;
}
