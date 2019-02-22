#ifndef __S_POINT_JOINT_D6_H__
#define __S_POINT_JOINT_D6_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/joints/point_joint_d6.h"

class DLLSERVER PointJointD6
	: public SBaseEntity,
	public BasePointJointD6
{
protected:

public:
	PointJointD6();
	virtual void OnSpawn() override;
	void SetKeyValue(std::string key,std::string val);
};
#endif

#endif