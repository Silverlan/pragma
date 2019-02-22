#include "stdafx_client.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/c_point_joint_distance.h"
#include "pragma/entities/c_entityfactories.h"
LINK_ENTITY_TO_CLASS(point_joint_distance,CPointJointDistance);
CPointJointDistance::CPointJointDistance()
	: CBaseEntity(),BasePointJointDistance()
{}

void CPointJointDistance::Spawn()
{
	CBaseEntity::Spawn();
}
#endif