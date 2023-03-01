/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/util_best_fitting_plane.hpp"
#include <Eigen/Eigenvalues>

void umath::calc_best_fitting_plane(const Mat3 &covarianceMatrix, const Vector3 &average, Vector3 &n, double &d)
{
	auto &A = reinterpret_cast<const Eigen::Matrix3f &>(covarianceMatrix);
	Eigen::EigenSolver<Eigen::Matrix3f> es(A);
	auto &eigenValues = es.eigenvalues();
	auto eigenVectors = es.eigenvectors();
	assert(eigenValues.size() == 3 && eigenVectors.size() == 3);
	std::array<float, 3> eigenRealValues = {eigenValues[0].real(), eigenValues[1].real(), eigenValues[2].real()};
	if(eigenRealValues.at(0) < eigenRealValues.at(1) && eigenRealValues.at(0) < eigenRealValues.at(2)) {
		auto v0 = eigenVectors.col(1);
		auto v1 = eigenVectors.col(2);
		n = uvec::cross({v0.x().real(), v0.y().real(), v0.z().real()}, {v1.x().real(), v1.y().real(), v1.z().real()});
	}
	else if(eigenRealValues.at(1) < eigenRealValues.at(0) && eigenRealValues.at(1) < eigenRealValues.at(2)) {
		auto v0 = eigenVectors.col(0);
		auto v1 = eigenVectors.col(2);
		n = uvec::cross({v0.x().real(), v0.y().real(), v0.z().real()}, {v1.x().real(), v1.y().real(), v1.z().real()});
	}
	else {
		auto v0 = eigenVectors.col(0);
		auto v1 = eigenVectors.col(1);
		n = uvec::cross({v0.x().real(), v0.y().real(), v0.z().real()}, {v1.x().real(), v1.y().real(), v1.z().real()});
	}
	d = uvec::dot(n, average);
}
