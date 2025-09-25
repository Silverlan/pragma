// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <mathutil/umat.h>

export module pragma.shared:math.best_fitting_plane;

export namespace umath {
	DLLNETWORK void calc_best_fitting_plane(const Mat3 &covarianceMatrix, const Vector3 &average, Vector3 &n, double &d);
};
