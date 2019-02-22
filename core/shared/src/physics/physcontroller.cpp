#include "stdafx_shared.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/physshape.h"
#include "pragma/physics/physcontroller.h"

DEFINE_BASE_HANDLE(DLLNETWORK,PhysController,PhysController);

PhysController::PhysController(PhysEnv *env)
	: PhysBase(env),m_ghostObject(),m_shape()
{
	m_handle = std::unique_ptr<PhysControllerHandle>(new PhysControllerHandle(this));
}

PhysController::PhysController(PhysEnv *env,float halfWidth,float halfHeight,float stepHeight,const PhysTransform &startTransform)
	: PhysController(env)
{
	m_shape = env->CreateCapsuleShape(halfWidth,halfHeight);
	m_ghostObject = std::unique_ptr<PhysGhostObject>(env->CreateGhostObject(std::static_pointer_cast<PhysShape>(m_shape)));
	m_ghostObject->SetWorldTransform(startTransform);

	m_ghostObject->SetCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	m_ghostObject->SetContactProcessingThreshold(btScalar(0.0));
	
	btPairCachingGhostObject *ghostObject = m_ghostObject->GetGhostObject();
	btConvexShape *btShape = m_shape->GetConvexShape();
	
	m_controller = std::unique_ptr<PhysKinematicCharacterController>(new PhysKinematicCharacterController(ghostObject,btShape,btScalar(stepHeight *PhysEnv::WORLD_SCALE)));
	m_controller->setGravity({0.0,0.0,0.0});
	m_controller->setUseGhostSweepTest(false); // If set to true => causes penetration issues with convex meshes, resulting in bouncy physics

	auto *world = env->GetWorld();
	world->addAction(m_controller.get());
}

PhysController::PhysController(PhysEnv *env,const Vector3 &halfExtents,float stepHeight,const PhysTransform &startTransform)
	: PhysController(env)
{
	m_shape = env->CreateBoxShape(halfExtents);
	m_ghostObject = std::unique_ptr<PhysGhostObject>(env->CreateGhostObject(std::static_pointer_cast<PhysShape>(m_shape)));
	m_ghostObject->SetWorldTransform(startTransform);

	m_ghostObject->SetCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	m_ghostObject->SetContactProcessingThreshold(btScalar(0.0));
	
	btPairCachingGhostObject *ghostObject = m_ghostObject->GetGhostObject();
	btConvexShape *btShape = m_shape->GetConvexShape();
	
	m_controller = std::unique_ptr<PhysKinematicCharacterController>(new PhysKinematicCharacterController(ghostObject,btShape,btScalar(stepHeight *PhysEnv::WORLD_SCALE)));
	m_controller->setGravity({0.0,0.0,0.0});
	m_controller->setUseGhostSweepTest(false); // If set to true => causes penetration issues with convex meshes, resulting in bouncy physics

	auto *world = env->GetWorld();
	world->addAction(m_controller.get());
}

PhysController::~PhysController()
{
	m_physEnv->RemoveController(this);
	m_handle->Invalidate();
	auto *world = m_physEnv->GetWorld();
	world->removeAction(m_controller.get());
}

float PhysController::GetMaxSlope() const {return CFloat(umath::rad_to_deg(CFloat(m_controller->getMaxSlope())));}
void PhysController::SetMaxSlope(float slope) {m_controller->setMaxSlope(umath::deg_to_rad(slope));}

std::shared_ptr<PhysConvexShape> PhysController::GetShape() {return m_shape;}
Vector3 PhysController::GetDimensions() const
{
	if(m_shape == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto *shape = m_shape.get();
	auto *btShape = static_cast<btConvexInternalShape*>(shape->GetShape());
	auto dimensions = btShape->getImplicitShapeDimensions() /PhysEnv::WORLD_SCALE;
	return Vector3(dimensions.x(),dimensions.y(),dimensions.z());
}
void PhysController::SetDimensions(const Vector3 &dimensions)
{
	if(m_shape == nullptr)
		return;
	auto *shape = m_shape.get();
	auto *btShape = static_cast<btConvexInternalShape*>(shape->GetShape());
	btShape->setImplicitShapeDimensions(btVector3(dimensions.x,dimensions.y,dimensions.z) *PhysEnv::WORLD_SCALE);
	auto *world = m_physEnv->GetWorld();
	world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(m_ghostObject->GetGhostObject()->getBroadphaseHandle(),world->getDispatcher());
}

PhysGhostObject *PhysController::GetGhostObject() {return m_ghostObject.get();}

PhysControllerHandle *PhysController::CreateHandle() {return m_handle->Copy();}
PhysControllerHandle PhysController::GetHandle() {return *m_handle;}

PhysKinematicCharacterController *PhysController::GetCharacterController() {return m_controller.get();}

void PhysController::SetWalkDirection(Vector3 &disp)
{
	m_controller->setWalkDirection(btVector3(disp.x,disp.y,disp.z) *PhysEnv::WORLD_SCALE);
}

Vector3 PhysController::GetWalkDirection() const
{
	return uvec::create(m_controller->getWalkDirection() /PhysEnv::WORLD_SCALE);
}
