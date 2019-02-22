#ifndef __C_POINT_JOINT_PRISMATIC_H__
#define __C_POINT_JOINT_PRISMATIC_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/joints/point_joint_prismatic.h"

class DLLCLIENT CPointJointPrismatic
	: public CBaseEntity,
	public BasePointJointPrismatic
{
public:
	CPointJointPrismatic();
	void Spawn();
};
#endif

#endif