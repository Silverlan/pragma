#ifndef __C_POINT_JOINT_D6_H__
#define __C_POINT_JOINT_D6_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/joints/point_joint_d6.h"

class DLLCLIENT CPointJointD6
	: public CBaseEntity,
	public BasePointJointD6
{
public:
	CPointJointD6();
	void Spawn();
};
#endif

#endif