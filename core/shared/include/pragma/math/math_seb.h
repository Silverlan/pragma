/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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