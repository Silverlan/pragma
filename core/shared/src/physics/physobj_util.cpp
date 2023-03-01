/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/physics/collision_object.hpp"
#include <pragma/game/game.h>
#include "pragma/entities/components/base_physics_component.hpp"

PhysObjKinematic::PhysObjKinematic() : m_bKinematic(false) {}
void PhysObjKinematic::SetKinematic(bool b) { m_bKinematic = b; }

void PhysObjDynamic::PreSimulate()
{
	auto *phys = dynamic_cast<PhysObj *>(this);
	for(auto &hObj : phys->GetCollisionObjects()) {
		if(hObj.IsValid() == false || hObj->IsRigid() == false)
			continue;
		hObj->GetRigidBody()->PreSimulate();
	}
}
void PhysObjDynamic::PostSimulate()
{
	auto *phys = dynamic_cast<PhysObj *>(this);
	for(auto &hObj : phys->GetCollisionObjects()) {
		if(hObj.IsValid() == false || hObj->IsRigid() == false)
			continue;
		hObj->GetRigidBody()->PostSimulate();
	}
}

bool PhysObjKinematic::IsKinematic() const { return false; }

////////////////////////////////////

#if PHYS_KEEP_SIMULATION_TRANSFORM != 0
void PhysObjDynamic::PreSimulate()
{
	PhysObj *phys = dynamic_cast<PhysObj *>(this);
	std::vector<PhysCollisionObject *> &objs = phys->GetCollisionObjects();
	if(m_offsets.size() != objs.size())
		m_offsets.resize(objs.size());
	for(unsigned int i = 0; i < objs.size(); i++) {
		PhysCollisionObject *o = objs[i];
		m_offsets[i] = o->GetWorldTransform();
	}
}
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
		return Vector3(0, 0, 0);
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
#endif
