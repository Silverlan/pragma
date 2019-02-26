#include "quaternion48.h"

Quaternion48::Quaternion48()
{}
Quat Quaternion48::operator()() const
{
	Quat tmp {};
	tmp.x = ((int)x -32'768) *(1 /32'768.0);
	tmp.y = ((int)y -32'768) *(1 /32'768.0);
	tmp.z = ((int)z -16'384) *(1 /16'384.0);
	tmp.w = sqrt(1 -tmp.x *tmp.x -tmp.y *tmp.y -tmp.z *tmp.z);
	if(wneg)
		tmp.w = -tmp.w;
	return tmp;
}
