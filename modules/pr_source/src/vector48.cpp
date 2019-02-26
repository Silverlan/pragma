#include "vector48.h"

Vector48::Vector48()
{
	x.Init();
	y.Init();
	z.Init();
}
Vector3 Vector48::ToVector3() const {return {x.GetFloat(),y.GetFloat(),z.GetFloat()};}
