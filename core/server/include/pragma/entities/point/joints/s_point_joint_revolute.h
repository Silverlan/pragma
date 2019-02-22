#ifndef __S_POINT_JOINT_REVOLUTE_H__
#define __S_POINT_JOINT_REVOLUTE_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/joints/point_joint_revolute.h"

class DLLSERVER PointJointRevolute
	: public SBaseEntity,
	public BasePointJointRevolute
{
protected:

public:
	PointJointRevolute();
	virtual void OnSpawn() override;
	void SetKeyValue(std::string key,std::string val);
};
#endif

#endif