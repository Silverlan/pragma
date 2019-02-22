#ifndef __CONTROLLERHITDATA_H__
#define __CONTROLLERHITDATA_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

class PhysObjHandle;
struct DLLNETWORK ControllerHitData
{
	ControllerHitData();
	~ControllerHitData();
	PhysObjHandle *physObj = nullptr;
	Vector3 hitNormal = {};
	void Clear();
};

#endif