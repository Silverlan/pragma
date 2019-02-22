#ifndef __C_POINT_JOINT_REVOLUTE_H__
#define __C_POINT_JOINT_REVOLUTE_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/joints/point_joint_revolute.h"

class DLLCLIENT CPointJointRevolute
	: public CBaseEntity,
	public BasePointJointRevolute
{
public:
	CPointJointRevolute();
	void Spawn();
};
#endif

#endif