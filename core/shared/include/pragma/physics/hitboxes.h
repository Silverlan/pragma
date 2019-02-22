#ifndef __HITBOXES_H__
#define __HITBOXES_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/uvec.h>
#include <limits>

enum class DLLNETWORK HitGroup : uint32_t
{
	Invalid = std::numeric_limits<std::underlying_type_t<HitGroup>>::max(),
	Generic = 0,
	Head = 1,
	Chest = 2,
	Stomach = 3,
	LeftArm = 4,
	RightArm = 5,
	LeftLeg = 6,
	RightLeg = 7,
	Gear = 8,
	Tail = 9
};

struct DLLNETWORK Hitbox
{
	Hitbox(HitGroup _group,const Vector3 &_min,const Vector3 &_max)
		: group(_group),min(_min),max(_max)
	{}
	HitGroup group;
	Vector3 min;
	Vector3 max;
};

#endif
