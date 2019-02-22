#include "stdafx_client.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/c_point_joint_fixed.h"
#include "pragma/entities/c_entityfactories.h"
LINK_ENTITY_TO_CLASS(point_joint_fixed,CPointJointFixed);
CPointJointFixed::CPointJointFixed()
	: CBaseEntity(),BasePointJointFixed()
{}

void CPointJointFixed::Spawn()
{
	CBaseEntity::Spawn();
}
#endif