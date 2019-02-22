#ifndef __C_POINT_JOINT_DISTANCE_H__
#define __C_POINT_JOINT_DISTANCE_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/joints/point_joint_distance.h"

class DLLCLIENT CPointJointDistance
	: public CBaseEntity,
	public BasePointJointDistance
{
public:
	CPointJointDistance();
	void Spawn();
};
#endif

#endif