#ifndef __S_POINT_JOINT_SPHERICAL_H__
#define __S_POINT_JOINT_SPHERICAL_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/joints/point_joint_spherical.h"

class DLLSERVER PointJointSpherical
	: public SBaseEntity,
	public BasePointJointSpherical
{
protected:

public:
	PointJointSpherical();
	virtual void OnSpawn() override;
	void SetKeyValue(std::string key,std::string val);
};
#endif

#endif