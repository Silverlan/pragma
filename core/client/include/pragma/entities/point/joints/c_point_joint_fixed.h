#ifndef __C_POINT_JOINT_FIXED_H__
#define __C_POINT_JOINT_FIXED_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/joints/point_joint_fixed.h"

class DLLCLIENT CPointJointFixed
	: public CBaseEntity,
	public BasePointJointFixed
{
public:
	CPointJointFixed();
	void Spawn();
};
#endif

#endif