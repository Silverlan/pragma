// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.orientation;

using namespace pragma;

OrientationComponent::OrientationComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_upDirection(util::Vector3Property::Create({0.f, 1.f, 0.f})) {}
void OrientationComponent::Initialize() { BaseEntityComponent::Initialize(); }
void OrientationComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
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

	//m_axRot = uquat::create(uvec::PRM_FORWARD,uvec::PRM_RIGHT,m_upDirection);//uvec::PRM_UP);

	m_axRot = uvec::get_rotation(Vector3(0.f, 1.f, 0.f), *m_upDirection);
	uquat::normalize(m_axRot);
	m_axForward = Vector3(0.f, 0.f, 1.f) * m_axRot;
	m_axRight = Vector3(-1.f, 0.f, 0.f) * m_axRot;

	//auto &ent = GetEntity();
	//PhysObj *phys = ent.GetPhysicsObject();
	//if(phys == nullptr || !phys->IsController())
	//	return;
}
