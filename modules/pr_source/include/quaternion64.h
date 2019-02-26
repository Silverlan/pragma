#ifndef __QUATERNION64_H__
#define __QUATERNION64_H__

#include <mathutil/uvec.h>
#include <mathutil/uquat.h>

#pragma pack(push,1)
class Quaternion64
{
private:
	uint64_t x:21;
	uint64_t y:21;
	uint64_t z:21;
	uint64_t wneg:1;
public:
	Quaternion64();
	Quat operator()() const;
};
#pragma pack(pop)

#endif
