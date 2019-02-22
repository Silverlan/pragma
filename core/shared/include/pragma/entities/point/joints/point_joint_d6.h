#ifndef __POINT_JOINT_D6_H__
#define __POINT_JOINT_D6_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/joints/point_joint_base.h"
class DLLNETWORK BasePointJointD6
	: public BasePointJoint
{
protected:
	BasePointJointD6();
};
#endif

#endif