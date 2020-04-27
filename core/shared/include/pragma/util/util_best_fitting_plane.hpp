/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_BEST_FITTING_PLANE_HPP__
#define __UTIL_BEST_FITTING_PLANE_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/umat.h>

namespace umath
{
	void calc_best_fitting_plane(const Mat3 &covarianceMatrix,const Vector3 &average,Vector3 &n,double &d);
};

#endif
