#ifndef __POINT_JOINT_FIXED_H__
#define __POINT_JOINT_FIXED_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/joints/point_joint_base.h"
class DLLNETWORK BasePointJointFixed
	: public BasePointJoint
{
protected:
	BasePointJointFixed();
};
#endif

#endif