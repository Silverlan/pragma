#ifndef __DELTAORIENTATION_H__
#define __DELTAORIENTATION_H__

#include "pragma/networkdefinitions.h"
#include <pragma/math/angle/wvquaternion.h>

struct DLLNETWORK DeltaOrientation
{
	DeltaOrientation(Quat porientation,double ptime,double pdelta=0.f);
	double time = 0.0;
	double delta = 0.0;
	Quat orientation = uquat::identity();
};

#endif