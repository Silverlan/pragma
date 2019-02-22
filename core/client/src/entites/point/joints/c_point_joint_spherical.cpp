#include "stdafx_client.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/c_point_joint_spherical.h"
#include "pragma/entities/c_entityfactories.h"
LINK_ENTITY_TO_CLASS(point_joint_spherical,CPointJointSpherical);
CPointJointSpherical::CPointJointSpherical()
	: CBaseEntity(),BasePointJointSpherical()
{}

void CPointJointSpherical::Spawn()
{
	CBaseEntity::Spawn();
}
#endif