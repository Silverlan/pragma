#ifndef __VECTOR48_H__
#define __VECTOR48_H__

#include <mathutil/uvec.h>
#include "float16.h"

#pragma pack(push,1)
class Vector48
{
public:
	Vector48();
	float16 x;
	float16 y;
	float16 z;
	Vector3 ToVector3() const;
};
#pragma pack(pop)

#endif
