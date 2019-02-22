#ifndef __BULLETAPI_H__
#define __BULLETAPI_H__

#include "pragma/definitions.h"
#include "pragma/math/vector/wvvector3.h"
#pragma warning(disable: 4127)
#pragma warning(disable: 4100)
#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#pragma warning(default: 4100)
#pragma warning(default: 4127)

namespace uvec
{
	DLLENGINE Vector3 create(const btVector3 &v);
	DLLENGINE btVector3 create_bt(const Vector3 &v);
};

namespace uquat
{
	DLLENGINE Quat create(const btQuaternion &q);
	DLLENGINE btQuaternion create_bt(const Quat &q);
};

#endif