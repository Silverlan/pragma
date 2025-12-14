// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.object;

pragma::physics::PhysObjKinematic::PhysObjKinematic() : m_bKinematic(false) {}
void pragma::physics::PhysObjKinematic::SetKinematic(bool b) { m_bKinematic = b; }

void pragma::physics::PhysObjDynamic::PreSimulate()
{
	auto *phys = dynamic_cast<PhysObj *>(this);
	for(auto &hObj : phys->GetCollisionObjects()) {
		if(hObj.IsValid() == false || hObj->IsRigid() == false)
			continue;
		hObj->GetRigidBody()->PreSimulate();
	}
}
void pragma::physics::PhysObjDynamic::PostSimulate()
{
	auto *phys = dynamic_cast<PhysObj *>(this);
	for(auto &hObj : phys->GetCollisionObjects()) {
		if(hObj.IsValid() == false || hObj->IsRigid() == false)
			continue;
		hObj->GetRigidBody()->PostSimulate();
	}
}

bool pragma::physics::PhysObjKinematic::IsKinematic() const { return false; }

////////////////////////////////////

#if PHYS_KEEP_SIMULATION_TRANSFORM != 0
void pragma::physics::PhysObjDynamic::PreSimulate()
{
	pragma::physics::PhysObj *phys = dynamic_cast<pragma::physics::PhysObj *>(this);
	std::vector<PhysCollisionObject *> &objs = phys->GetCollisionObjects();
	if(m_offsets.size() != objs.size())
		m_offsets.resize(objs.size());
	for(unsigned int i = 0; i < objs.size(); i++) {
		PhysCollisionObject *o = objs[i];
		m_offsets[i] = o->GetWorldTransform();
	}
}
void pragma::physics::PhysObjDynamic::PostSimulate()
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
Vector3 pragma::physics::PhysObjDynamic::GetSimulationOffset(unsigned int idx)
{
	if(idx >= m_offsets.size())
		return Vector3(0, 0, 0);
	PhysTransform &t = m_offsets[idx];
	return t.GetOrigin();
}
Quat pragma::physics::PhysObjDynamic::GetSimulationRotation(unsigned int idx)
{
	if(idx >= m_offsets.size())
		return uquat::identity();
	PhysTransform &t = m_offsets[idx];
	return t.GetRotation();
}
#endif
