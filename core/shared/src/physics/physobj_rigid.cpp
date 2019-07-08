#include "stdafx_shared.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/physics/collision_object.hpp"
#include <pragma/game/game.h>
#include "pragma/physics/environment.hpp"

RigidPhysObj::RigidPhysObj(pragma::BaseEntityComponent *owner)
	: PhysObj(owner),PhysObjKinematic(),PhysObjDynamic()
{}
bool RigidPhysObj::Initialize(pragma::physics::IRigidBody &body)
{
	if(PhysObj::Initialize() == false)
		return false;
	AddCollisionObject(body);
	m_mass = body.GetMass();
	m_rigidBodies.push_back(util::shared_handle_cast<pragma::physics::IBase,pragma::physics::IRigidBody>(body.ClaimOwnership()));
	return true;
}
bool RigidPhysObj::Initialize(const std::vector<pragma::physics::IRigidBody*> &bodies)
{
	if(PhysObj::Initialize() == false)
		return false;
	m_mass = 0.f;
	auto first = true;
	m_rigidBodies.reserve(bodies.size());
	for(auto *body : bodies)
	{
		AddCollisionObject(*body);
		m_rigidBodies.push_back(util::shared_handle_cast<pragma::physics::IBase,pragma::physics::IRigidBody>(body->ClaimOwnership()));
		if(first)
		{
			m_mass = body->GetMass();
			first = false;
		}
	}
	return true;
}
RigidPhysObj::~RigidPhysObj()
{}
void RigidPhysObj::SetDamping(float linDamping,float angDamping)
{
	SetLinearDamping(linDamping);
	SetAngularDamping(angDamping);
}
void RigidPhysObj::SetLinearDamping(float damping)
{
	for(auto it=m_rigidBodies.begin();it!=m_rigidBodies.end();++it)
	{
		auto &body = *it;
		if(body.IsValid())
			body->SetLinearDamping(damping);
	}
}
void RigidPhysObj::SetAngularDamping(float damping)
{
	for(auto it=m_rigidBodies.begin();it!=m_rigidBodies.end();++it)
	{
		auto &body = *it;
		if(body.IsValid())
			body->SetAngularDamping(damping);
	}
}
float RigidPhysObj::GetLinearDamping() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return PhysObj::GetLinearDamping();
	return body->GetLinearDamping();
}
float RigidPhysObj::GetAngularDamping() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return PhysObj::GetAngularDamping();
	return body->GetAngularDamping();
}
std::vector<util::TSharedHandle<pragma::physics::IRigidBody>> &RigidPhysObj::GetRigidBodies() {return m_rigidBodies;}
void RigidPhysObj::UpdateVelocity()
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	m_velocity = body->GetLinearVelocity();
	/*PhysRigidBody *body = GetRigidBody();
	if(body == NULL)
		return Vector3(0.f,0.f,0.f);
	return body->GetLinearVelocity();*/
	// m_velocity = 
	/*if(m_owner == NULL || !m_owner->IsValid())
		return;
	NetworkState *state = (*m_owner)->GetNetworkState();
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
void RigidPhysObj::Simulate(double tDelta,bool bIgnoreGravity) {}
void RigidPhysObj::SetKinematic(bool bKinematic)
{
	for(auto &hRigidBody : m_rigidBodies)
	{
		if(hRigidBody.IsValid() == false)
			continue;
		hRigidBody->SetKinematic(bKinematic);
	}
}
pragma::BaseEntityComponent *RigidPhysObj::GetOwner() {return PhysObj::GetOwner();}
pragma::physics::IRigidBody *RigidPhysObj::GetRigidBody()
{
	if(m_rigidBodies.empty())
		return nullptr;
	auto &hBody = m_rigidBodies.front();
	if(!hBody.IsValid())
		return nullptr;
	return hBody.Get();
}
const pragma::physics::IRigidBody *RigidPhysObj::GetRigidBody() const {return const_cast<RigidPhysObj*>(this)->GetRigidBody();}
void RigidPhysObj::ApplyMass(float mass)
{
	for(unsigned int i=0;i<m_rigidBodies.size();i++)
	{
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			body->GetRigidBody()->SetMass(mass);
	}
}
float RigidPhysObj::GetMass() const {return m_mass;}
void RigidPhysObj::SetMass(float mass)
{
	m_mass = mass;
	if(IsStatic())
		return;
	ApplyMass(mass);
}
void RigidPhysObj::SetLinearFactor(const Vector3 &factor)
{
	for(auto it=m_rigidBodies.begin();it!=m_rigidBodies.end();++it)
	{
		auto &hBody = *it;
		if(hBody.IsValid())
			hBody->GetRigidBody()->SetLinearFactor(factor);
	}
}
void RigidPhysObj::SetAngularFactor(const Vector3 &factor)
{
	for(auto it=m_rigidBodies.begin();it!=m_rigidBodies.end();++it)
	{
		auto &hBody = *it;
		if(hBody.IsValid())
			hBody->GetRigidBody()->SetAngularFactor(factor);
	}
}
Vector3 RigidPhysObj::GetLinearFactor() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f,0.f,0.f);
	return body->GetLinearFactor();
}
Vector3 RigidPhysObj::GetAngularFactor() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f,0.f,0.f);
	return body->GetAngularFactor();
}

void RigidPhysObj::ApplyForce(const Vector3 &force)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyForce(force);
}
void RigidPhysObj::ApplyForce(const Vector3 &force,const Vector3 &relPos)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyForce(force,relPos);
}
void RigidPhysObj::ApplyImpulse(const Vector3 &impulse)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyImpulse(impulse);
}
void RigidPhysObj::ApplyImpulse(const Vector3 &impulse,const Vector3 &relPos)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyImpulse(impulse,relPos);
}
void RigidPhysObj::ApplyTorque(const Vector3 &torque)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyTorque(torque);
}
void RigidPhysObj::ApplyTorqueImpulse(const Vector3 &torque)
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ApplyTorqueImpulse(torque);
}
void RigidPhysObj::ClearForces()
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return;
	body->ClearForces();
}
Vector3 RigidPhysObj::GetTotalForce() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f,0.f,0.f);
	return body->GetTotalForce();
}
Vector3 RigidPhysObj::GetTotalTorque() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f,0.f,0.f);
	return body->GetTotalTorque();
}
void RigidPhysObj::SetSleepingThresholds(float linear,float angular)
{
	for(auto &hBody : m_rigidBodies)
	{
		if(hBody.IsValid() == false)
			continue;
		hBody->SetSleepingThresholds(linear,angular);
	}
}
float RigidPhysObj::GetLinearSleepingThreshold() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return 0.f;
	return body->GetLinearSleepingThreshold();
}
float RigidPhysObj::GetAngularSleepingThreshold() const
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return 0.f;
	return body->GetAngularSleepingThreshold();
}

bool RigidPhysObj::IsStatic() const {return m_bStatic;}
void RigidPhysObj::SetStatic(bool b)
{
	m_bStatic = b;
	for(unsigned int i=0;i<m_rigidBodies.size();i++)
	{
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			body->SetStatic(b);
	}
	if(b == true)
		ApplyMass(0.f);
	else
		ApplyMass(m_mass);
}
bool RigidPhysObj::IsRigid() const {return true;}
void RigidPhysObj::AddCollisionObject(pragma::physics::ICollisionObject &o)
{
	if(o.IsRigid() == false)
		return;
	PhysObj::AddCollisionObject(o);
	m_rigidBodies.push_back(util::shared_handle_cast<pragma::physics::IBase,pragma::physics::IRigidBody>(o.ClaimOwnership()));
}
Vector3 RigidPhysObj::GetLinearVelocity() const
{
	return m_velocity;
}
void RigidPhysObj::SetLinearVelocity(const Vector3 &vel)
{
	for(unsigned int i=0;i<m_rigidBodies.size();i++)
	{
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
		{
			body->SetLinearVelocity(vel);
			body->WakeUp();
		}
	}
}
Vector3 RigidPhysObj::GetAngularVelocity() const
{
	auto *body = GetRigidBody();
	if(body == NULL)
		return Vector3(0.f,0.f,0.f);
	return body->GetAngularVelocity();
}
void RigidPhysObj::SetAngularVelocity(const Vector3 &vel)
{
	for(unsigned int i=0;i<m_rigidBodies.size();i++)
	{
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
		{
			body->SetAngularVelocity(vel);
			body->WakeUp();
		}
	}
}
void RigidPhysObj::PutToSleep()
{
	for(unsigned int i=0;i<m_rigidBodies.size();i++)
	{
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			body->PutToSleep();
	}
}
void RigidPhysObj::WakeUp()
{
	for(unsigned int i=0;i<m_rigidBodies.size();i++)
	{
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			body->WakeUp();
	}
}
bool RigidPhysObj::IsSleeping() const
{
	auto *body = GetRigidBody();
	if(body == NULL)
		return true;
	return body->IsAsleep();
}
void RigidPhysObj::OnSleep()
{
	// BULLETTODO
}
void RigidPhysObj::OnWake()
{
	 // BULLETTODO
}
