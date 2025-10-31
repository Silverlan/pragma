// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;




module pragma.shared;

import :physics.soft_body_info;

SoftBodyPhysObj::SoftBodyPhysObj(pragma::BaseEntityComponent *owner) : pragma::physics::PhysObj(owner), PhysObjDynamic() {}
bool SoftBodyPhysObj::Initialize(pragma::physics::ISoftBody &body)
{
	if(pragma::physics::PhysObj::Initialize() == false)
		return false;
	AddCollisionObject(body);
	m_softBodies.push_back(util::shared_handle_cast<pragma::physics::IBase, pragma::physics::ISoftBody>(body.ClaimOwnership()));
	return true;
}
bool SoftBodyPhysObj::Initialize(const std::vector<pragma::physics::ISoftBody *> &bodies)
{
	if(pragma::physics::PhysObj::Initialize() == false)
		return false;
	m_softBodies.reserve(bodies.size());
	for(auto *body : bodies) {
		AddCollisionObject(*body);
		m_softBodies.push_back(util::shared_handle_cast<pragma::physics::IBase, pragma::physics::ISoftBody>(body->ClaimOwnership()));
	}
	return true;
}
void SoftBodyPhysObj::SetLinearVelocity(const Vector3 &vel)
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid()) {
			auto *body = hBody.Get();
			body->SetLinearVelocity(vel);
			body->WakeUp();
		}
	}
}
std::vector<util::TSharedHandle<pragma::physics::ISoftBody>> &SoftBodyPhysObj::GetSoftBodies() { return m_softBodies; }
const pragma::physics::ISoftBody *SoftBodyPhysObj::GetSoftBody() const { return const_cast<SoftBodyPhysObj *>(this)->GetSoftBody(); }
pragma::physics::ISoftBody *SoftBodyPhysObj::GetSoftBody()
{
	if(m_softBodies.empty())
		return nullptr;
	auto &hBody = m_softBodies.front();
	if(!hBody.IsValid())
		return nullptr;
	return hBody.Get();
}
bool SoftBodyPhysObj::IsStatic() const { return false; }
bool SoftBodyPhysObj::IsSoftBody() const { return true; }
void SoftBodyPhysObj::AddCollisionObject(pragma::physics::ICollisionObject &o)
{
	auto *body = o.GetSoftBody();
	if(body == nullptr)
		return;
	pragma::physics::PhysObj::AddCollisionObject(o);
	m_softBodies.push_back(util::shared_handle_cast<pragma::physics::IBase, pragma::physics::ISoftBody>(body->ClaimOwnership()));
}

float SoftBodyPhysObj::GetMass() const
{
	auto *body = GetSoftBody();
	if(body == nullptr)
		return 0.f;
	return body->GetMass();
}
void SoftBodyPhysObj::SetMass(float mass)
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid())
			hBody->SetMass(mass);
	}
}
pragma::BaseEntityComponent *SoftBodyPhysObj::GetOwner() { return m_owner.get(); }
void SoftBodyPhysObj::Simulate(double tDelta, bool bIgnoreGravity) {}
void SoftBodyPhysObj::PutToSleep()
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid())
			hBody->GetSoftBody()->SetActivationState(pragma::physics::ICollisionObject::ActivationState::WaitForDeactivation);
	}
}
void SoftBodyPhysObj::WakeUp()
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid())
			hBody->WakeUp();
	}
}
bool SoftBodyPhysObj::IsSleeping() const
{
	auto *body = GetSoftBody();
	if(body == nullptr)
		return true;
	return body->IsAsleep();
}

void SoftBodyPhysObj::ApplyForce(const Vector3 &force)
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid())
			hBody->AddForce(force);
	}
}
