#ifndef __DELTATRANSFORM_H__
#define __DELTATRANSFORM_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <pragma/math/angle/wvquaternion.h>

struct DLLNETWORK DeltaTransform
{
	DeltaTransform(Vector3 poffset,Quat porientation,double ptime,double pdelta=0.f);
	double time = 0.0;
	double delta = 0.0;
	Vector3 offset = {};
	Quat orientation = uquat::identity();
};

#endif