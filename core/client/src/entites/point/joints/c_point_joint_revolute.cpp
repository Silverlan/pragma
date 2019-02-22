#include "stdafx_client.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/c_point_joint_revolute.h"
#include "pragma/entities/c_entityfactories.h"
LINK_ENTITY_TO_CLASS(point_joint_revolute,CPointJointRevolute);
CPointJointRevolute::CPointJointRevolute()
	: CBaseEntity(),BasePointJointRevolute()
{}

void CPointJointRevolute::Spawn()
{
	CBaseEntity::Spawn();
}
#endif