#ifndef __POINT_JOINT_REVOLUTE_H__
#define __POINT_JOINT_REVOLUTE_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/joints/point_joint_base.h"
class DLLNETWORK BasePointJointRevolute
	: public BasePointJoint
{
protected:
	BasePointJointRevolute();
};
#endif

#endif