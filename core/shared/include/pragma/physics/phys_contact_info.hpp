#ifndef __PHYS_CONTACT_INFO_HPP__
#define __PHYS_CONTACT_INFO_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>

// Obsolete?
// TODO: Remove this file
struct DLLNETWORK PhysContactInfo
{
	static Vector3 GetContactNormal(const Vector3 &n,int8_t controllerIndex);
	static double CalcXZDistance(const Vector3 &contactPointA,const Vector3 &contactPointB,int8_t controllerIndex);

	PhysContactInfo(int8_t controllerIndex);
	Vector3 GetContactNormal() const;
	double CalcXZDistance() const;
	util::TWeakSharedHandle<pragma::physics::ICollisionObject> contactObject0 = {}; // The character controller
	util::TWeakSharedHandle<pragma::physics::ICollisionObject> contactObject1 = {};
	int32_t surfaceMaterialId = -1;
	int8_t controllerIndex = -1; // = 0 if the controller is object 0 in 'contactPoint', otherwise 1

	Vector3 contactPointA = {};
	Vector3 contactPointB = {};
	Vector3 normalWorldOnB = {};
	float contactDistance = std::numeric_limits<float>::max();
};

#endif
