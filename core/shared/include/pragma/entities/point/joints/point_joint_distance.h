#ifndef __POINT_JOINT_DISTANCE_H__
#define __POINT_JOINT_DISTANCE_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/joints/point_joint_base.h"
class DLLNETWORK BasePointJointDistance
	: public BasePointJoint
{
protected:
	BasePointJointDistance();
};
#endif

#endif