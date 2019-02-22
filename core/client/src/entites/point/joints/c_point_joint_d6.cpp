#include "stdafx_client.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/c_point_joint_d6.h"
#include "pragma/entities/c_entityfactories.h"
LINK_ENTITY_TO_CLASS(point_joint_d6,CPointJointD6);
CPointJointD6::CPointJointD6()
	: CBaseEntity(),BasePointJointD6()
{}

void CPointJointD6::Spawn()
{
	CBaseEntity::Spawn();
}
#endif