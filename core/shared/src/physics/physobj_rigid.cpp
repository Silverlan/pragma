#include "stdafx_shared.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/physics/physcollisionobject.h"
#include <pragma/game/game.h>
#include "pragma/physics/physenvironment.h"

RigidPhysObj::RigidPhysObj(pragma::BaseEntityComponent *owner,PhysRigidBody *body)
	: PhysObj(owner,body),PhysObjKinematic(),PhysObjDynamic(),
	m_mass(body->GetMass())
{
	m_rigidBodies.push_back(body->GetHandle());
}
RigidPhysObj::RigidPhysObj(pragma::BaseEntityComponent *owner,std::vector<PhysRigidBody*> *bodies)
	: PhysObj(owner),PhysObjKinematic(),PhysObjDynamic(),
	m_mass(0.f)
{
	for(unsigned int i=0;i<bodies->size();i++)
	{
		AddCollisionObject((*bodies)[i]);
		m_rigidBodies.push_back((*bodies)[i]->GetHandle());
		if(i == 0)
			m_mass = (*bodies)[i]->GetMass();
	}
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
			static_cast<PhysRigidBody*>(body.get())->SetLinearDamping(damping);
	}
}
void RigidPhysObj::SetAngularDamping(float damping)
{
	for(auto it=m_rigidBodies.begin();it!=m_rigidBodies.end();++it)
	{
		auto &body = *it;
		if(body.IsValid())
			static_cast<PhysRigidBody*>(body.get())->SetAngularDamping(damping);
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
std::vector<PhysRigidBodyHandle> &RigidPhysObj::GetRigidBodies() {return m_rigidBodies;}
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
		static_cast<PhysRigidBody*>(hRigidBody.get())->SetKinematic(bKinematic);
	}
}
pragma::BaseEntityComponent *RigidPhysObj::GetOwner() {return PhysObj::GetOwner();}
PhysRigidBody *RigidPhysObj::GetRigidBody() const
{
	if(m_rigidBodies.empty())
		return nullptr;
	auto &hBody = m_rigidBodies.front();
	if(!hBody.IsValid())
		return nullptr;
	return static_cast<PhysRigidBody*>(hBody.get());
}
void RigidPhysObj::ApplyMass(float mass)
{
	for(unsigned int i=0;i<m_rigidBodies.size();i++)
	{
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			static_cast<PhysRigidBody*>(body.get())->SetMass(mass);
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
			static_cast<PhysRigidBody*>(hBody.get())->SetLinearFactor(factor);
	}
}
void RigidPhysObj::SetAngularFactor(const Vector3 &factor)
{
	for(auto it=m_rigidBodies.begin();it!=m_rigidBodies.end();++it)
	{
		auto &hBody = *it;
		if(hBody.IsValid())
			static_cast<PhysRigidBody*>(hBody.get())->SetAngularFactor(factor);
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
Vector3 RigidPhysObj::GetTotalForce()
{
	auto *body = GetRigidBody();
	if(body == nullptr)
		return Vector3(0.f,0.f,0.f);
	return body->GetTotalForce();
}
Vector3 RigidPhysObj::GetTotalTorque()
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
		static_cast<PhysRigidBody*>(hBody.get())->SetSleepingThresholds(linear,angular);
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
		{
			int flags = body->GetCollisionFlags();
			if(b == true)
				flags |= btCollisionObject::CF_STATIC_OBJECT;
			else
				flags &= ~btCollisionObject::CF_STATIC_OBJECT;
			body->SetCollisionFlags(flags);
		}
	}
	if(b == true)
		ApplyMass(0.f);
	else
		ApplyMass(m_mass);
}
bool RigidPhysObj::IsRigid() const {return true;}
void RigidPhysObj::AddCollisionObject(PhysCollisionObject *o)
{
	PhysRigidBody *body = dynamic_cast<PhysRigidBody*>(o);
	if(body == NULL)
		return;
	PhysObj::AddCollisionObject(o);
	m_rigidBodies.push_back(body->GetHandle());
}
Vector3 RigidPhysObj::GetLinearVelocity()
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
			auto *rigid = static_cast<PhysRigidBody*>(body.get());
			rigid->SetLinearVelocity(vel);
			rigid->Activate();
		}
	}
}
Vector3 RigidPhysObj::GetAngularVelocity()
{
	PhysRigidBody *body = GetRigidBody();
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
			auto *rigid = static_cast<PhysRigidBody*>(body.get());
			rigid->SetAngularVelocity(vel);
			rigid->Activate();
		}
	}
}
void RigidPhysObj::PutToSleep()
{
	for(unsigned int i=0;i<m_rigidBodies.size();i++)
	{
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			static_cast<PhysRigidBody*>(body.get())->SetActivationState(WANTS_DEACTIVATION);
	}
}
void RigidPhysObj::WakeUp()
{
	for(unsigned int i=0;i<m_rigidBodies.size();i++)
	{
		auto &body = m_rigidBodies[i];
		if(body.IsValid())
			static_cast<PhysRigidBody*>(body.get())->Activate();
	}
}
bool RigidPhysObj::IsSleeping()
{
	PhysRigidBody *body = GetRigidBody();
	if(body == NULL)
		return true;
	int state = body->GetActivationState();
	return (state == DISABLE_SIMULATION || state == ISLAND_SLEEPING) ? true : false;
}
void RigidPhysObj::OnSleep()
{
	// BULLETTODO
}
void RigidPhysObj::OnWake()
{
	 // BULLETTODO
}
