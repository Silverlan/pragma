#ifndef __BT_PHYSENVIRONMENT_H__
#define __BT_PHYSENVIRONMENT_H__

#include "pragma/networkdefinitions.h"
#include <pragma/physics/physapi.h>

/* Defined in bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.cpp */
struct DLLNETWORK btBridgedManifoldResultCustom : public btManifoldResult
{

	btCollisionWorld::ContactResultCallback&	m_resultCallback;

	btBridgedManifoldResultCustom( const btCollisionObjectWrapper* obj0Wrap,const btCollisionObjectWrapper* obj1Wrap,btCollisionWorld::ContactResultCallback& resultCallback );

	virtual void addContactPoint(const btVector3& normalOnBInWorld,const btVector3& pointInWorld,btScalar depth);
	
};
struct DLLNETWORK btSingleContactCallbackCustom : public btBroadphaseAabbCallback
{

	btCollisionObject* m_collisionObject;
	btCollisionWorld*	m_world;
	btCollisionWorld::ContactResultCallback&	m_resultCallback;
	
	
	btSingleContactCallbackCustom(btCollisionObject* collisionObject, btCollisionWorld* world,btCollisionWorld::ContactResultCallback& resultCallback);

	virtual bool	process(const btBroadphaseProxy* proxy);
};

#endif