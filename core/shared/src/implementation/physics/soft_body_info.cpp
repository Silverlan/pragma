// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.soft_body_info;

pragma::physics::SoftBodyPhysObj::SoftBodyPhysObj(BaseEntityComponent *owner) : PhysObj(owner), PhysObjDynamic() {}
bool pragma::physics::SoftBodyPhysObj::Initialize(ISoftBody &body)
{
	if(PhysObj::Initialize() == false)
		return false;
	AddCollisionObject(body);
	m_softBodies.push_back(pragma::util::shared_handle_cast<IBase, ISoftBody>(body.ClaimOwnership()));
	return true;
}
bool pragma::physics::SoftBodyPhysObj::Initialize(const std::vector<ISoftBody *> &bodies)
{
	if(PhysObj::Initialize() == false)
		return false;
	m_softBodies.reserve(bodies.size());
	for(auto *body : bodies) {
		AddCollisionObject(*body);
		m_softBodies.push_back(pragma::util::shared_handle_cast<IBase, ISoftBody>(body->ClaimOwnership()));
	}
	return true;
}
void pragma::physics::SoftBodyPhysObj::SetLinearVelocity(const Vector3 &vel)
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid()) {
			auto *body = hBody.Get();
			body->SetLinearVelocity(vel);
			body->WakeUp();
		}
	}
}
std::vector<pragma::util::TSharedHandle<pragma::physics::ISoftBody>> &pragma::physics::SoftBodyPhysObj::GetSoftBodies() { return m_softBodies; }
const pragma::physics::ISoftBody *pragma::physics::SoftBodyPhysObj::GetSoftBody() const { return const_cast<SoftBodyPhysObj *>(this)->GetSoftBody(); }
pragma::physics::ISoftBody *pragma::physics::SoftBodyPhysObj::GetSoftBody()
{
	if(m_softBodies.empty())
		return nullptr;
	auto &hBody = m_softBodies.front();
	if(!hBody.IsValid())
		return nullptr;
	return hBody.Get();
}
bool pragma::physics::SoftBodyPhysObj::IsStatic() const { return false; }
bool pragma::physics::SoftBodyPhysObj::IsSoftBody() const { return true; }
void pragma::physics::SoftBodyPhysObj::AddCollisionObject(ICollisionObject &o)
{
	auto *body = o.GetSoftBody();
	if(body == nullptr)
		return;
	PhysObj::AddCollisionObject(o);
	m_softBodies.push_back(pragma::util::shared_handle_cast<IBase, ISoftBody>(body->ClaimOwnership()));
}

float pragma::physics::SoftBodyPhysObj::GetMass() const
{
	auto *body = GetSoftBody();
	if(body == nullptr)
		return 0.f;
	return body->GetMass();
}
void pragma::physics::SoftBodyPhysObj::SetMass(float mass)
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid())
			hBody->SetMass(mass);
	}
}
pragma::BaseEntityComponent *pragma::physics::SoftBodyPhysObj::GetOwner() { return m_owner.get(); }
void pragma::physics::SoftBodyPhysObj::Simulate(double tDelta, bool bIgnoreGravity) {}
void pragma::physics::SoftBodyPhysObj::PutToSleep()
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid())
			hBody->GetSoftBody()->SetActivationState(ICollisionObject::ActivationState::WaitForDeactivation);
	}
}
void pragma::physics::SoftBodyPhysObj::WakeUp()
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid())
			hBody->WakeUp();
	}
}
bool pragma::physics::SoftBodyPhysObj::IsSleeping() const
{
	auto *body = GetSoftBody();
	if(body == nullptr)
		return true;
	return body->IsAsleep();
}

void pragma::physics::SoftBodyPhysObj::ApplyForce(const Vector3 &force)
{
	for(auto &hBody : m_softBodies) {
		if(hBody.IsValid())
			hBody->AddForce(force);
	}
}
