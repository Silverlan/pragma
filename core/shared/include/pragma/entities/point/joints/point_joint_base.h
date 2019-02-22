#ifndef __POINT_JOINT_BASE_H__
#define __POINT_JOINT_BASE_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <string>
#include <vector>
class BaseEntity;
class DLLNETWORK BasePointJoint
{
protected:
	BasePointJoint();
	std::string m_kvSource;
	std::string m_kvTarget;
	Vector3 m_posTarget = {0.f,0.f,0.f};
	virtual void SetKeyValue(std::string key,std::string val) override;
	void GetTargetEntities(std::vector<BaseEntity*> &entsSrc,std::vector<BaseEntity*> &entsTgt);
};
#endif

#endif
