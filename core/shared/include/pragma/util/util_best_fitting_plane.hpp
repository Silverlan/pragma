// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_BEST_FITTING_PLANE_HPP__
#define __UTIL_BEST_FITTING_PLANE_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/umat.h>

namespace umath {
	DLLNETWORK void calc_best_fitting_plane(const Mat3 &covarianceMatrix, const Vector3 &average, Vector3 &n, double &d);
};

#endif
