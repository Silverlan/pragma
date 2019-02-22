#include "stdafx_shared.h"
#include "pragma/math/deltatransform.h"

DeltaTransform::DeltaTransform(Vector3 poffset,Quat porientation,double ptime,double pdelta)
	: offset(poffset),orientation(porientation),time(ptime),delta(pdelta)
{}