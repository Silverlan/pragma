#ifndef __QUATERNION48_H__
#define __QUATERNION48_H__

#include <mathutil/uvec.h>

#pragma pack(push,1)
class Quaternion48
{
private:
	uint16_t x:16;
	uint16_t y:16;
	uint16_t z:15;
	uint16_t wneg:1;
public:
	Quaternion48();
	Quat operator()() const;
};
#pragma pack(pop)

#endif
