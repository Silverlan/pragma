#ifndef __PHYSAPI_H__
#define __PHYSAPI_H__

#ifdef PHYS_ENGINE_PHYSX
	#include "pragma/physics/physxapi.h"
#endif
#ifdef PHYS_ENGINE_BULLET
	#include "pragma/physics/bulletapi.h"
#endif

#endif