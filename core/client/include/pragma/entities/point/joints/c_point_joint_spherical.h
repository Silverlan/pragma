#ifndef __C_POINT_JOINT_SPHERICAL_H__
#define __C_POINT_JOINT_SPHERICAL_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/joints/point_joint_spherical.h"

class DLLCLIENT CPointJointSpherical
	: public CBaseEntity,
	public BasePointJointSpherical
{
public:
	CPointJointSpherical();
	void Spawn();
};
#endif

#endif