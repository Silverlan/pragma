#include "stdafx_shared.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/physics/physcollisionobject.h"
#include <pragma/game/game.h>
#include "pragma/physics/physenvironment.h"

SoftBodyPhysObj::SoftBodyPhysObj(pragma::BaseEntityComponent *owner,PhysSoftBody *body)
	: PhysObj(owner,body),PhysObjDynamic()
{
	m_softBodies.push_back(body->GetHandle());
}
SoftBodyPhysObj::SoftBodyPhysObj(pragma::BaseEntityComponent *owner,std::vector<PhysSoftBody*> *bodies)
	: PhysObj(owner),PhysObjDynamic()
{
	m_softBodies.reserve(bodies->size());
	for(auto *body : *bodies)
	{
		AddCollisionObject(body);
		m_softBodies.push_back(body->GetHandle());
	}
}
void SoftBodyPhysObj::SetLinearVelocity(const Vector3 &vel)
{
	for(auto &hBody : m_softBodies)
	{
		if(hBody.IsValid())
		{
			auto *body = static_cast<PhysSoftBody*>(hBody.get());
			body->SetLinearVelocity(vel);
			body->Activate();
		}
	}
}
std::vector<PhysSoftBodyHandle> &SoftBodyPhysObj::GetSoftBodies() {return m_softBodies;}
PhysSoftBody *SoftBodyPhysObj::GetSoftBody() const
{
	if(m_softBodies.empty())
		return nullptr;
	auto &hBody = m_softBodies.front();
	if(!hBody.IsValid())
		return nullptr;
	return static_cast<PhysSoftBody*>(hBody.get());
}
bool SoftBodyPhysObj::IsStatic() const {return false;}
bool SoftBodyPhysObj::IsSoftBody() const {return true;}
void SoftBodyPhysObj::AddCollisionObject(PhysCollisionObject *o)
{
	auto *body = dynamic_cast<PhysSoftBody*>(o);
	if(body == nullptr)
		return;
	PhysObj::AddCollisionObject(o);
	m_softBodies.push_back(body->GetHandle());
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
	for(auto &hBody : m_softBodies)
	{
		if(hBody.IsValid())
			static_cast<PhysSoftBody*>(hBody.get())->SetMass(mass);
	}
}
pragma::BaseEntityComponent *SoftBodyPhysObj::GetOwner() {return m_owner.get();}
void SoftBodyPhysObj::Simulate(double tDelta,bool bIgnoreGravity) {}
void SoftBodyPhysObj::PutToSleep()
{
	for(auto &hBody : m_softBodies)
	{
		if(hBody.IsValid())
			static_cast<PhysSoftBody*>(hBody.get())->SetActivationState(WANTS_DEACTIVATION);
	}
}
void SoftBodyPhysObj::WakeUp()
{
	for(auto &hBody : m_softBodies)
	{
		if(hBody.IsValid())
			static_cast<PhysSoftBody*>(hBody.get())->Activate();
	}
}
bool SoftBodyPhysObj::IsSleeping()
{
	auto *body = GetSoftBody();
	if(body == nullptr)
		return true;
	auto state = body->GetActivationState();
	return (state == DISABLE_SIMULATION || state == ISLAND_SLEEPING) ? true : false;
}

void SoftBodyPhysObj::ApplyForce(const Vector3 &force)
{
	for(auto &hBody : m_softBodies)
	{
		if(hBody.IsValid())
			static_cast<PhysSoftBody*>(hBody.get())->AddForce(force);
	}
}

