#ifndef __PHYSCONTACT_H__
#define __PHYSCONTACT_H__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <mathutil/glmutil.h>
#include <pragma/physics/physapi.h>

class BaseEntity;
class PhysObj;
class PhysCollisionObject;
class DLLNETWORK PhysContact
{
public:
	PhysContact();
	BaseEntity *entA = nullptr;
	BaseEntity *entB = nullptr;
	PhysObj *physA = nullptr;
	PhysObj* physB = nullptr;
	PhysCollisionObject *objA = nullptr;
	PhysCollisionObject *objB = nullptr;
	Vector3 posA = {};
	Vector3 posB = {};
	Vector3 hitNormal = {};
};

#endif