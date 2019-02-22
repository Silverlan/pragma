#ifndef __S_POINT_JOINT_DISTANCE_H__
#define __S_POINT_JOINT_DISTANCE_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/joints/point_joint_distance.h"

class DLLSERVER PointJointDistance
	: public SBaseEntity,
	public BasePointJointDistance
{
protected:

public:
	PointJointDistance();
	virtual void OnSpawn() override;
	void SetKeyValue(std::string key,std::string val);
};
#endif

#endif