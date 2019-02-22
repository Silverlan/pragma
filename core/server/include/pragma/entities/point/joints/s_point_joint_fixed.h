#ifndef __S_POINT_JOINT_FIXED_H__
#define __S_POINT_JOINT_FIXED_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/joints/point_joint_fixed.h"

class DLLSERVER PointJointFixed
	: public SBaseEntity,
	public BasePointJointFixed
{
public:
	PointJointFixed();
	virtual void OnSpawn() override;
	void SetKeyValue(std::string key,std::string val);
};
#endif

#endif