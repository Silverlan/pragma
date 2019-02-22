#ifndef __MATH_SEB_H__
#define __MATH_SEB_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <vector>

namespace Seb
{
	void DLLNETWORK Calculate(std::vector<Vector3> &verts,Vector3 &center,float &radius);
};

#endif