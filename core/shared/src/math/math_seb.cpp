#include "stdafx_shared.h"
#include "pragma/math/math_seb.h"
#include "Seb.h"

void Seb::Calculate(std::vector<Vector3> &verts,Vector3 &center,float &radius)
{
	int d = 3;
	std::vector<Seb::Point<double>> points;
	for(unsigned int i=0;i<verts.size();i++)
	{
		Vector3 &v = verts[i];
		std::vector<double> vec(3);
		vec[0] = v.x;
		vec[1] = v.y;
		vec[2] = v.z;
		points.push_back(Seb::Point<double>(3,vec.begin()));
	}
	Seb::Smallest_enclosing_ball<double> seb(d,points);
	radius = static_cast<float>(seb.radius());
	Seb::Smallest_enclosing_ball<double>::Coordinate_iterator itCenter = seb.center_begin();
	center.x = static_cast<float>(itCenter[0]);
	center.y = static_cast<float>(itCenter[1]);
	center.z = static_cast<float>(itCenter[2]);
}
