#ifndef __PHYSOVERLAPFILTERCALLBACK_H__
#define __PHYSOVERLAPFILTERCALLBACK_H__

#include "pragma/networkdefinitions.h"
#include <pragma/physics/physapi.h>

class PhysCollisionObject;
class DLLNETWORK PhysOverlapFilterCallback
	: public btOverlapFilterCallback
{
	virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0,btBroadphaseProxy* proxy1) const override;
};

#endif