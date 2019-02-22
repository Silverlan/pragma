#ifndef __POINT_JOINT_SPHERICAL_H__
#define __POINT_JOINT_SPHERICAL_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/joints/point_joint_base.h"
class DLLNETWORK BasePointJointSpherical
	: public BasePointJoint
{
protected:
	BasePointJointSpherical();
};
#endif

#endif