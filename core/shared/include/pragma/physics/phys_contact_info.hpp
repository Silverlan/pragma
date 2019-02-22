#ifndef __PHYS_CONTACT_INFO_HPP__
#define __PHYS_CONTACT_INFO_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/physcollisionobject.h"
#include <cinttypes>
#include <BulletCollision/NarrowPhaseCollision/btManifoldPoint.h>

struct DLLNETWORK PhysContactInfo
{
	static Vector3 GetContactNormal(const Vector3 &n,int8_t controllerIndex);
	static double CalcXZDistance(const btManifoldPoint &contactPoint,int8_t controllerIndex);

	PhysContactInfo(const btManifoldPoint &contactPoint,int8_t controllerIndex);
	Vector3 GetContactNormal() const;
	double CalcXZDistance() const;
	btManifoldPoint contactPoint {};
	PhysCollisionObjectHandle contactObject0 {}; // The character controller
	PhysCollisionObjectHandle contactObject1 {};
	int32_t surfaceMaterialId = -1;
	int8_t controllerIndex = -1; // = 0 if the controller is object 0 in 'contactPoint', otherwise 1
};

#endif
