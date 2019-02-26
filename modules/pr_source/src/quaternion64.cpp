#include "quaternion64.h"

Quaternion64::Quaternion64()
{}

Quat Quaternion64::operator()() const
{
	Quat tmp {};
	// shift to -1048576, + 1048575, then round down slightly to -1.0 < x < 1.0
	tmp.x = ((int)x -1'048'576) *(1 /1'048'576.5f);
	tmp.y = ((int)y -1'048'576) *(1 /1'048'576.5f);
	tmp.z = ((int)z -1'048'576) *(1 /1'048'576.5f);
	tmp.w = sqrt(1 -tmp.x *tmp.x -tmp.y *tmp.y -tmp.z *tmp.z);
	if(wneg)
		tmp.w = -tmp.w;
	return tmp; 
}
