// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.best_fitting_plane;

export import pragma.math;

export namespace pragma::math {
	DLLNETWORK void calc_best_fitting_plane(const Mat3 &covarianceMatrix, const Vector3 &average, Vector3 &n, double &d);
};
