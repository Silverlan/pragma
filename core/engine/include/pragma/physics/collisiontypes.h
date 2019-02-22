#ifndef __COLLISIONTYPE_H__
#define __COLLISIONTYPE_H__
#include "pragma/networkdefinitions.h"
enum class COLLISIONTYPE : int
{
	NONE,
	AABB,
	OBB,
	BRUSH
};
#endif