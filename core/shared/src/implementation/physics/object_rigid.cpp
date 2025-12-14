// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.object;

 pragma::physics::RigidPhysObj::RigidPhysObj(BaseEntityComponent *owner) : PhysObj(owner), PhysObjKinematic(), PhysObjDynamic() {}
bool  pragma::physics::RigidPhysObj::Initialize(IRigidBody &body)
{
	if(PhysObj::Initialize() == false)
		return false;
	AddCollisionObject(body);
	m_mass = body.GetMass();
	return true;
}
bool  pragma::physics::RigidPhysObj::Initialize(const std::vector<IRigidBody *> &bodies)
{
	if(PhysObj::Initialize() == false)
		return false;
	m_mass = 0.f;
	auto first = true;
	m_rigidBodies.reserve(bodies.size());
	for(auto *body : bodies) {
		AddCollisionObject(*body);
		if(first) {
			m_mass = body->GetMass();
			first = false;
		}
	}
	return true;
}
 pragma::physics::RigidPhysObj::~RigidPhysObj() {}
void  pragma::physics::RigidPhysObj::SetDamping(float linDamping, float angDamping)
{
	SetLinearDamping(linDamping);
	SetAngularDamping(angDamping);
}
void  pragma::physics::RigidPhysObj::SetLinearDamping(float damping)
{
	for(auto it = m_rigidBodies.begin(); it != m_rigidBodies.end(); ++it) {
		auto &body = *it;
		if(body.IsValid())
			body->SetLinearDamping(damping);
	}
}
void  pragma::physics::RigidPhysObj::SetAngularDamping(float damping)
{
	for(auto it = m_rigidBodies.begin(); it != m_rigidBodies.end(); ++it) {
		auto &body = *it;
		if(body.IsValid())
			body->SetAngularDamping(damping);
	}
}
float  pragma::physics::RigidPhysObj::GetLinearDamping() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return PhysObj::GetLinearDamping();
	return body->GetLinearDamping();
}
float  pragma::physics::RigidPhysObj::GetAngularDamping() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return PhysObj::GetAngularDamping();
	return body->GetAngularDamping();
}
std::vector<pragma::util::TSharedHandle<pragma::physics::IRigidBody>> & pragma::physics::RigidPhysObj::GetRigidBodies() { return m_rigidBodies; }
void  pragma::physics::RigidPhysObj::UpdateVelocity()
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	m_velocity = body->GetLinearVelocity();
	/*PhysRigidBody *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f,0.f,0.f);
	return body->GetLinearVelocity();*/
	// m_velocity =
	/*if(m_owner == nullptr || !m_owner->IsValid())
		return;
	auto *state = (*m_owner)->GetNetworkState();
	Game *game = state->GetGameState();
	double delta = game->DeltaTickTime();
	float scale;
	if(delta == 0)
		scale = 1;
	else
		scale = 1.f /delta;
	PhysTransform t = m_ghostObject->GetWorldTransform();
	Vector3 pos = t.GetOrigin();
	m_velocity = ((pos -m_posLast) -m_originLast) *scale;
	m_originLast = pos;
	m_posLast = Vector3(0.f,0.f,0.f);*/
}
void  pragma::physics::RigidPhysObj::Simulate(double tDelta, bool bIgnoreGravity) {}
void  pragma::physics::RigidPhysObj::SetKinematic(bool bKinematic)
{
	for(auto &hRigidBody : m_rigidBodies) {
		if(hRigidBody.IsValid() == false)
			continue;
		hRigidBody->SetKinematic(bKinematic);
	}
}
pragma::BaseEntityComponent * pragma::physics::RigidPhysObj::GetOwner() { return PhysObj::GetOwner(); }
pragma::physics::IRigidBody * pragma::physics::RigidPhysObj::GetRigidBody()
{
	if(m_rigidBodies.empty())
		return nullptr;
	auto &hBody = m_rigidBodies.front();
	if(!hBody.IsValid())
		return nullptr;
	return hBody.Get();
}
const pragma::physics::IRigidBody * pragma::physics::RigidPhysObj::GetRigidBody() const { return const_cast<RigidPhysObj *>(this)->GetRigidBody(); }
void  pragma::physics::RigidPhysObj::ApplyMass(float mass)
{
	for(unsigned int i = 0; i < m_rigidBodies.size(); i++) {
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			body->GetRigidBody()->SetMassAndUpdateInertia(mass);
	}
}
float  pragma::physics::RigidPhysObj::GetMass() const { return m_mass; }
void  pragma::physics::RigidPhysObj::SetMass(float mass)
{
	m_mass = mass;
	if(IsStatic())
		return;
	ApplyMass(mass);
}
void  pragma::physics::RigidPhysObj::SetLinearFactor(const Vector3 &factor)
{
	for(auto it = m_rigidBodies.begin(); it != m_rigidBodies.end(); ++it) {
		auto &hBody = *it;
		if(hBody.IsValid())
			hBody->GetRigidBody()->SetLinearFactor(factor);
	}
}
void  pragma::physics::RigidPhysObj::SetAngularFactor(const Vector3 &factor)
{
	for(auto it = m_rigidBodies.begin(); it != m_rigidBodies.end(); ++it) {
		auto &hBody = *it;
		if(hBody.IsValid())
			hBody->GetRigidBody()->SetAngularFactor(factor);
	}
}
Vector3  pragma::physics::RigidPhysObj::GetLinearFactor() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f, 0.f, 0.f);
	return body->GetLinearFactor();
}
Vector3  pragma::physics::RigidPhysObj::GetAngularFactor() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f, 0.f, 0.f);
	return body->GetAngularFactor();
}

void  pragma::physics::RigidPhysObj::ApplyForce(const Vector3 &force)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyForce(force);
}
void  pragma::physics::RigidPhysObj::ApplyForce(const Vector3 &force, const Vector3 &relPos)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyForce(force, relPos);
}
void  pragma::physics::RigidPhysObj::ApplyImpulse(const Vector3 &impulse)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyImpulse(impulse);
}
void  pragma::physics::RigidPhysObj::ApplyImpulse(const Vector3 &impulse, const Vector3 &relPos)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyImpulse(impulse, relPos);
}
void  pragma::physics::RigidPhysObj::ApplyTorque(const Vector3 &torque)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyTorque(torque);
}
void  pragma::physics::RigidPhysObj::ApplyTorqueImpulse(const Vector3 &torque)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyTorqueImpulse(torque);
}
void  pragma::physics::RigidPhysObj::ClearForces()
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ClearForces();
}
Vector3  pragma::physics::RigidPhysObj::GetTotalForce() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f, 0.f, 0.f);
	return body->GetTotalForce();
}
Vector3  pragma::physics::RigidPhysObj::GetTotalTorque() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f, 0.f, 0.f);
	return body->GetTotalTorque();
}
void  pragma::physics::RigidPhysObj::SetSleepingThresholds(float linear, float angular)
{
	for(auto &hBody : m_rigidBodies) {
		if(hBody.IsValid() == false)
			continue;
		hBody->SetSleepingThresholds(linear, angular);
	}
}
float  pragma::physics::RigidPhysObj::GetLinearSleepingThreshold() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return 0.f;
	return body->GetLinearSleepingThreshold();
}
float  pragma::physics::RigidPhysObj::GetAngularSleepingThreshold() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return 0.f;
	return body->GetAngularSleepingThreshold();
}

bool  pragma::physics::RigidPhysObj::IsStatic() const { return m_bStatic; }
void  pragma::physics::RigidPhysObj::SetStatic(bool b)
{
	m_bStatic = b;
	for(unsigned int i = 0; i < m_rigidBodies.size(); i++) {
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			body->SetStatic(b);
	}
	/* Mass mustn't be changed here, since changing it also affects center-of-mass and inertia!
	if(b == true)
		ApplyMass(0.f);
	else
		ApplyMass(m_mass);*/
}
bool  pragma::physics::RigidPhysObj::IsRigid() const { return true; }
void  pragma::physics::RigidPhysObj::AddCollisionObject(ICollisionObject &o)
{
	if(o.IsRigid() == false)
		return;
	PhysObj::AddCollisionObject(o);
	m_rigidBodies.push_back(pragma::util::shared_handle_cast<IBase, IRigidBody>(o.ClaimOwnership()));
}
Vector3  pragma::physics::RigidPhysObj::GetLinearVelocity() const { return m_velocity; }
void  pragma::physics::RigidPhysObj::SetLinearVelocity(const Vector3 &vel)
{
	for(unsigned int i = 0; i < m_rigidBodies.size(); i++) {
		auto &body = m_rigidBodies[i];
		if(body.IsValid()) {
			body->SetLinearVelocity(vel);
			body->WakeUp();
		}
	}
}
Vector3  pragma::physics::RigidPhysObj::GetAngularVelocity() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f, 0.f, 0.f);
	return body->GetAngularVelocity();
}
void  pragma::physics::RigidPhysObj::SetAngularVelocity(const Vector3 &vel)
{
	for(unsigned int i = 0; i < m_rigidBodies.size(); i++) {
		auto &body = m_rigidBodies[i];
		if(body.IsValid()) {
			body->SetAngularVelocity(vel);
			body->WakeUp();
		}
	}
}
void  pragma::physics::RigidPhysObj::PutToSleep()
{
	for(unsigned int i = 0; i < m_rigidBodies.size(); i++) {
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			body->PutToSleep();
	}
}
void  pragma::physics::RigidPhysObj::WakeUp()
{
	for(unsigned int i = 0; i < m_rigidBodies.size(); i++) {
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			body->WakeUp();
	}
}
bool  pragma::physics::RigidPhysObj::IsSleeping() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return true;
	return body->IsAsleep();
}
void  pragma::physics::RigidPhysObj::OnSleep() { PhysObj::OnSleep(); }
void  pragma::physics::RigidPhysObj::OnWake() { PhysObj::OnWake(); }
