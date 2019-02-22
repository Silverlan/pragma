#include "stdafx_shared.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/physics/physcollisionobject.h"
#include <pragma/game/game.h>
#include "pragma/entities/components/base_physics_component.hpp"

PhysObjKinematic::PhysObjKinematic()
	: m_bKinematic(false)
{}
void PhysObjKinematic::SetKinematic(bool b)
{
	m_bKinematic = b;
}

void PhysObjDynamic::PreSimulate()
{
	auto *phys = dynamic_cast<PhysObj*>(this);
	for(auto &hObj : phys->GetCollisionObjects())
	{
		if(hObj.IsValid() == false || hObj->IsRigid() == false)
			continue;
		static_cast<PhysRigidBody*>(hObj.get())->PreSimulate();
	}
}
void PhysObjDynamic::PostSimulate()
{
	auto *phys = dynamic_cast<PhysObj*>(this);
	for(auto &hObj : phys->GetCollisionObjects())
	{
		if(hObj.IsValid() == false || hObj->IsRigid() == false)
			continue;
		static_cast<PhysRigidBody*>(hObj.get())->PostSimulate();
	}
}

#ifdef PHYS_ENGINE_BULLET
bool PhysObjKinematic::IsKinematic() {return false;}
#elif PHYS_ENGINE_PHYSX
void PhysObjKinematic::SetKinematicTarget(const Vector3 &pos)
{
	PhysObj *phys = dynamic_cast<PhysObj*>(this);
	Vector3 offset = pos -phys->GetPosition();
	physx::PxVec3 pxOffset(offset.x,offset.y,offset.z);
	std::vector<physx::PxRigidActor*> *actors = phys->GetActors();
	for(unsigned int i=0;i<actors->size();i++)
	{
		physx::PxRigidDynamic *rigid = static_cast<physx::PxRigidDynamic*>((*actors)[i]);
		physx::PxTransform t = rigid->getGlobalPose();
		t.p = t.p +pxOffset;
		rigid->setKinematicTarget(t);
	}
}
void PhysObjKinematic::SetKinematicTarget(const Quat &rot)
{
	PhysObj *phys = dynamic_cast<PhysObj*>(this);
	Quat rotOffset = rot *phys->GetOrientation().GetInverse();
	physx::PxQuat pxRot(rotOffset.x,rotOffset.y,rotOffset.z,rotOffset.w);
	std::vector<physx::PxRigidActor*> *actors = phys->GetActors();
	for(unsigned int i=0;i<actors->size();i++)
	{
		physx::PxRigidDynamic *rigid = static_cast<physx::PxRigidDynamic*>((*actors)[i]);
		physx::PxTransform t = rigid->getGlobalPose();
		t.q = t.q *pxRot;
		rigid->setKinematicTarget(t);
	}
}
void PhysObjKinematic::SetKinematicTarget(const Vector3 &pos,const Quat &rot)
{
	PhysObj *phys = dynamic_cast<PhysObj*>(this);
	Vector3 offset = pos -phys->GetPosition();
	physx::PxVec3 pxOffset(offset.x,offset.y,offset.z);
	Quat rotOffset = rot *phys->GetOrientation().GetInverse();
	physx::PxQuat pxRot(rotOffset.x,rotOffset.y,rotOffset.z,rotOffset.w);
	std::vector<physx::PxRigidActor*> *actors = phys->GetActors();
	for(unsigned int i=0;i<actors->size();i++)
	{
		physx::PxRigidDynamic *rigid = static_cast<physx::PxRigidDynamic*>((*actors)[i]);
		physx::PxTransform t = rigid->getGlobalPose();
		t.p = t.p +pxOffset;
		t.q = t.q *pxRot;
		rigid->setKinematicTarget(t);
	}
}
bool PhysObjKinematic::IsKinematic() {return m_bKinematic;}

void PhysObjKinematic::SetKinematic(bool b,std::vector<physx::PxRigidActor*> &actors)
{
	m_bKinematic = b;
	for(unsigned int i=0;i<actors.size();i++)
	{
		if(actors[i]->isRigidDynamic())
		{
			physx::PxRigidDynamic *actor = static_cast<physx::PxRigidDynamic*>(actors[i]);
			actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC,true);
		}
	}
}
#endif

////////////////////////////////////

#if PHYS_KEEP_SIMULATION_TRANSFORM != 0
void PhysObjDynamic::PreSimulate()
{
	PhysObj *phys = dynamic_cast<PhysObj*>(this);
#ifdef PHYS_ENGINE_BULLET
	std::vector<PhysCollisionObject*> &objs = phys->GetCollisionObjects();
	if(m_offsets.size() != objs.size())
		m_offsets.resize(objs.size());
	for(unsigned int i=0;i<objs.size();i++)
	{
		PhysCollisionObject *o = objs[i];
		m_offsets[i] = o->GetWorldTransform();
	}
#elif PHYS_ENGINE_PHYSX
	std::vector<physx::PxRigidActor*> *actors = phys->GetActors();
	if(m_offsets.size() != actors->size())
		m_offsets.resize(actors->size());
	for(unsigned int i=0;i<actors->size();i++)
	{
		physx::PxRigidDynamic *rigid = static_cast<physx::PxRigidDynamic*>((*actors)[i]);
		m_offsets[i] = rigid->getGlobalPose();
	}
#endif
}
#ifdef PHYS_ENGINE_BULLET
void PhysObjDynamic::PostSimulate()
{
	/*PhysObj *phys = dynamic_cast<PhysObj*>(this);
	std::vector<PhysCollisionObject*> &objs = phys->GetCollisionObjects();
	for(unsigned int i=0;i<objs.size();i++)
	{
		PhysCollisionObject *o = objs[i];
		PhysTransform t = o->GetWorldTransform();
		PhysTransform &tOffset = m_offsets[i];
		Vector3 p = t.GetOrigin();
		Quat q = t.GetRotation();
		tOffset.SetOrigin(p -tOffset.GetOrigin());
		tOffset.SetRotation(q *tOffset.GetRotation().GetInverse());
		o->SetWorldTransform(tOffset);
	}*/
}
Vector3 PhysObjDynamic::GetSimulationOffset(unsigned int idx)
{
	if(idx >= m_offsets.size())
		return Vector3(0,0,0);
	PhysTransform &t = m_offsets[idx];
	return t.GetOrigin();
}
Quat PhysObjDynamic::GetSimulationRotation(unsigned int idx)
{
	if(idx >= m_offsets.size())
		return uquat::identity();
	PhysTransform &t = m_offsets[idx];
	return t.GetRotation();
}
#elif PHYS_ENGINE_PHYSX
void PhysObjDynamic::PostSimulate()
{
	PhysObj *phys = dynamic_cast<PhysObj*>(this);
	std::vector<physx::PxRigidActor*> *actors = phys->GetActors();
	for(unsigned int i=0;i<actors->size();i++)
	{
		physx::PxRigidDynamic *rigid = static_cast<physx::PxRigidDynamic*>((*actors)[i]);
		physx::PxTransform &t = rigid->getGlobalPose();
		physx::PxTransform &tOffset = m_offsets[i];
		tOffset.p = t.p -tOffset.p;
		tOffset.q = t.q *tOffset.q.getConjugate();
	}
}
Vector3 PhysObjDynamic::GetSimulationOffset(unsigned int idx)
{
	if(idx >= m_offsets.size())
		return Vector3(0,0,0);
	physx::PxTransform &t = m_offsets[idx];
	return Vector3(t.p.x,t.p.y,t.p.z);
}
Quat PhysObjDynamic::GetSimulationRotation(unsigned int idx)
{
	if(idx >= m_offsets.size())
		return Quat(1,0,0,0);
	physx::PxTransform &t = m_offsets[idx];
	return Quat(t.q.w,t.q.x,t.q.y,t.q.z);
}
#endif
#endif
