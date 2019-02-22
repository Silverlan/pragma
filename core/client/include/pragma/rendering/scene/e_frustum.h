#ifndef __E_FRUSTUM_H__
#define __E_FRUSTUM_H__

#include "pragma/clientdefinitions.h"

#undef NEAR
#undef FAR
enum class DLLCLIENT FRUSTUM_PLANE : int
{
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
	NEAR,
	FAR,
	COUNT
};

enum class DLLCLIENT FRUSTUM_POINT : int
{
	FAR_BOTTOM_LEFT,
	FAR_TOP_LEFT,
	FAR_TOP_RIGHT,
	FAR_BOTTOM_RIGHT,
	NEAR_BOTTOM_LEFT,
	NEAR_TOP_LEFT,
	NEAR_TOP_RIGHT,
	NEAR_BOTTOM_RIGHT
};

#endif