#ifndef __S_POINT_JOINT_PRISMATIC_H__
#define __S_POINT_JOINT_PRISMATIC_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/joints/point_joint_prismatic.h"

class DLLSERVER PointJointPrismatic
	: public SBaseEntity,
	public BasePointJointPrismatic
{
protected:

public:
	PointJointPrismatic();
	virtual void OnSpawn() override;
	void SetKeyValue(std::string key,std::string val);
};
#endif

#endif