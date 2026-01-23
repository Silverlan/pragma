// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "Seb.h"

module pragma.shared;

import :math.seb;

void Seb::Calculate(std::vector<Vector3> &verts, Vector3 &center, float &radius)
{
	int d = 3;
	std::vector<Point<double>> points;
	for(unsigned int i = 0; i < verts.size(); i++) {
		Vector3 &v = verts[i];
		std::vector<double> vec(3);
		vec[0] = v.x;
		vec[1] = v.y;
		vec[2] = v.z;
		points.push_back(Seb::Point<double>(3, vec.begin()));
	}
	Smallest_enclosing_ball<double> seb(d, points);
	radius = static_cast<float>(seb.radius());
	Smallest_enclosing_ball<double>::Coordinate_iterator itCenter = seb.center_begin();
	center.x = static_cast<float>(itCenter[0]);
	center.y = static_cast<float>(itCenter[1]);
	center.z = static_cast<float>(itCenter[2]);
}
