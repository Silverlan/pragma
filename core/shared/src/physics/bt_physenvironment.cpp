#include "stdafx_shared.h"
#include "pragma/physics/bt_physenvironment.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physshape.h"
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.h>

/* Defined in bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.cpp */

extern PhysEnv *g_simEnvironment; // Current simulated environment

static btScalar calculateCombinedFriction(float friction0,float friction1)
{
	//if(vehicle)
	//	return 0.f; // TODO
	btScalar friction = friction0 *friction1;

	const btScalar MAX_FRICTION = 10.f;
	if(friction < -MAX_FRICTION)
		friction = -MAX_FRICTION;
	if(friction > MAX_FRICTION)
		friction = MAX_FRICTION;
	return friction;
}

static btScalar calculateCombinedRestitution(float restitution0,float restitution1)
{
	return restitution0 *restitution1;
}

extern ContactAddedCallback gContactAddedCallback;
static bool get_collision_object_properties(Game *game,const btCollisionObject *o,float *friction,float *restitution)
{
	*friction = 1.f;
	*restitution = 1.f;
	auto *obj = static_cast<PhysCollisionObject*>(o->getUserPointer());
	if(obj == nullptr)
		return false;
	auto *mat = game->GetSurfaceMaterial(obj->GetSurfaceMaterial());
	if(mat == nullptr)
		return false;
	*friction = mat->GetFriction();
	*restitution = mat->GetRestitution();
	return true;
}

void _btAdjustInternalEdgeContacts(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap,const btCollisionObjectWrapper* colObj1Wrap, int partId0, int index0, int normalAdjustFlags=0);
static bool custom_material_combiner_callback(btManifoldPoint &cp,const btCollisionObjectWrapper *colObj0Wrapper,int partId0,int index0,const btCollisionObjectWrapper *colObj1Wrapper,int partId1,int index1)
{
	// TODO: This can crash in some instances
	//_btAdjustInternalEdgeContacts(cp,colObj1Wrapper,colObj0Wrapper,partId1,index1);

	auto *nw = g_simEnvironment->GetNetworkState();
	auto *game = nw->GetGameState();

	auto type0 = colObj0Wrapper->getCollisionShape()->getShapeType();
	auto type1 = colObj1Wrapper->getCollisionShape()->getShapeType();
	if(type0 == TRIANGLE_SHAPE_PROXYTYPE)
	{
		auto *parent0 = colObj0Wrapper->getCollisionObject()->getCollisionShape();
		if(parent0 != 0 && parent0->getShapeType() == MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE)
		{
			auto *shape = const_cast<btMultimaterialTriangleMeshShape*>(static_cast<const btMultimaterialTriangleMeshShape*>(parent0));
			auto *props = shape->getMaterialProperties(partId0,index0);
			float friction1,restitution1;
			get_collision_object_properties(game,colObj1Wrapper->getCollisionObject(),&friction1,&restitution1);
			cp.m_combinedFriction = calculateCombinedFriction(CFloat(props->m_friction),friction1);
			cp.m_combinedRestitution = calculateCombinedRestitution(CFloat(props->m_restitution),restitution1);
		}
	}
	else if(type1 == TRIANGLE_SHAPE_PROXYTYPE)
	{
		const btCollisionShape* parent1 = colObj1Wrapper->getCollisionObject()->getCollisionShape();
		if(parent1 != 0 && parent1->getShapeType() == MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE)
		{
			auto *shape = const_cast<btMultimaterialTriangleMeshShape*>(static_cast<const btMultimaterialTriangleMeshShape*>(parent1));
			auto *props = shape->getMaterialProperties(partId1,index1);
			float friction0,restitution0;
			get_collision_object_properties(game,colObj0Wrapper->getCollisionObject(),&friction0,&restitution0);
			cp.m_combinedFriction = calculateCombinedFriction(CFloat(props->m_friction),friction0);
			cp.m_combinedRestitution = calculateCombinedRestitution(CFloat(props->m_restitution),restitution0);
		}
	}
	else
	{
		auto *colObj0 = colObj0Wrapper->getCollisionObject();
		auto *colObj1 = colObj1Wrapper->getCollisionObject();
		float friction0,friction1;
		float restitution0,restitution1;
		get_collision_object_properties(game,colObj0,&friction0,&restitution0);
		get_collision_object_properties(game,colObj1,&friction1,&restitution1);
		/*if(colObj0->getCollisionFlags() &btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
		{
			friction0 = 1.0;//partId0,index0
			restitution0 = 0.f;
		}
		if(colObj1->getCollisionFlags() &btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
		{
			if(index1&1)
				friction1 = 1.0f;//partId1,index1
			else
				friction1 = 0.f;
			restitution1 = 0.f;
		}*/
		cp.m_combinedFriction = calculateCombinedFriction(CFloat(friction0),CFloat(friction1));
		cp.m_combinedRestitution = calculateCombinedRestitution(CFloat(restitution0),CFloat(restitution1));
	}

	cp.m_combinedFriction *= game->GetFrictionScale();
	cp.m_combinedRestitution *= game->GetRestitutionScale();

	//this return value is currently ignored, but to be on the safe side: return false if you don't calculate friction
	return true;
}

void btInitCustomMaterialCombinerCallback()
{
	gContactAddedCallback = custom_material_combiner_callback; // See http://www.bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=4603&start=0 ; Without this 'fix' vehicle physics will not work properly on triangle meshes; Also see 'physshape.cpp':109
}

//////////////////////////////

btBridgedManifoldResultCustom::btBridgedManifoldResultCustom(const btCollisionObjectWrapper *obj0Wrap,const btCollisionObjectWrapper *obj1Wrap,btCollisionWorld::ContactResultCallback &resultCallback)
	: btManifoldResult(obj0Wrap,obj1Wrap),m_resultCallback(resultCallback)
{}

void btBridgedManifoldResultCustom::addContactPoint(const btVector3 &normalOnBInWorld,const btVector3 &pointInWorld,btScalar depth)
{
	bool isSwapped = m_manifoldPtr->getBody0() != m_body0Wrap->getCollisionObject();
	btVector3 pointA = pointInWorld +normalOnBInWorld *depth;
	btVector3 localA;
	btVector3 localB;
	if(isSwapped)
	{
		localA = m_body1Wrap->getCollisionObject()->getWorldTransform().invXform(pointA);
		localB = m_body0Wrap->getCollisionObject()->getWorldTransform().invXform(pointInWorld);
	}
	else
	{
		localA = m_body0Wrap->getCollisionObject()->getWorldTransform().invXform(pointA);
		localB = m_body1Wrap->getCollisionObject()->getWorldTransform().invXform(pointInWorld);
	}
		
	btManifoldPoint newPt(localA,localB,normalOnBInWorld,depth);
	newPt.m_positionWorldOnA = pointA;
	newPt.m_positionWorldOnB = pointInWorld;
		
	//BP mod, store contact triangles.
	if(isSwapped)
	{
		newPt.m_partId0 = m_partId1;
		newPt.m_partId1 = m_partId0;
		newPt.m_index0  = m_index1;
		newPt.m_index1  = m_index0;
	}
	else
	{
		newPt.m_partId0 = m_partId0;
		newPt.m_partId1 = m_partId1;
		newPt.m_index0  = m_index0;
		newPt.m_index1  = m_index1;
	}

	//experimental feature info, for per-triangle material etc.
	const btCollisionObjectWrapper *obj0Wrap = isSwapped? m_body1Wrap : m_body0Wrap;
	const btCollisionObjectWrapper *obj1Wrap = isSwapped? m_body0Wrap : m_body1Wrap;
	m_resultCallback.addSingleResult(newPt,obj0Wrap,newPt.m_partId0,newPt.m_index0,obj1Wrap,newPt.m_partId1,newPt.m_index1);

}
btSingleContactCallbackCustom::btSingleContactCallbackCustom(btCollisionObject *collisionObject, btCollisionWorld *world,btCollisionWorld::ContactResultCallback &resultCallback)
	: m_collisionObject(collisionObject),
	m_world(world),
	m_resultCallback(resultCallback)
{}

bool btSingleContactCallbackCustom::process(const btBroadphaseProxy* proxy)
{
	btCollisionObject *collisionObject = (btCollisionObject*)proxy->m_clientObject;
	if(collisionObject == m_collisionObject)
		return true;

	//only perform raycast if filterMask matches
	if(m_resultCallback.needsCollision(collisionObject->getBroadphaseHandle())) 
	{
		btCollisionObjectWrapper ob0(0,m_collisionObject->getCollisionShape(),m_collisionObject,m_collisionObject->getWorldTransform(),-1,-1);
		btCollisionObjectWrapper ob1(0,collisionObject->getCollisionShape(),collisionObject,collisionObject->getWorldTransform(),-1,-1);

		btCollisionAlgorithm* algorithm = m_world->getDispatcher()->findAlgorithm(&ob0,&ob1,0,BT_CLOSEST_POINT_ALGORITHMS);
		if(algorithm)
		{
			btBridgedManifoldResultCustom contactPointResult(&ob0,&ob1,m_resultCallback);
			//discrete collision detection query
				
			algorithm->processCollision(&ob0,&ob1,m_world->getDispatchInfo(),&contactPointResult);

			algorithm->~btCollisionAlgorithm();
			m_world->getDispatcher()->freeCollisionAlgorithm(algorithm);
		}
	}
	return true;
}
