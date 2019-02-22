#include "stdafx_shared.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physconstraint.h"
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/physshape.h"
#include "pragma/physics/physcontroller.h"
#include <pragma/console/conout.h>
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/physcontact.h"
#include "pragma/physics/raytraces.h"
#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include <BulletCollision/CollisionDispatch/btCollisionObjectWrapper.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include "pragma/physics/bt_physenvironment.h"
#include "pragma/physics/physoverlapfiltercallback.h"
#include "pragma/audio/alsound_type.h"
#include "pragma/model/modelmesh.h"
#include "pragma/util/util_game.hpp"
#include "pragma/physics/phys_water_buoyancy_simulator.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"

const double PhysEnv::WORLD_SCALE = util::units_to_metres(1.0);
const double PhysEnv::WORLD_SCALE_SQR = umath::pow(PhysEnv::WORLD_SCALE,2.0);
const float PhysEnv::CCD_MOTION_THRESHOLD = 4.f *static_cast<float>(WORLD_SCALE);
const float PhysEnv::CCD_SWEPT_SPHERE_RADIUS = 2.f *static_cast<float>(WORLD_SCALE);
static const float PHYS_CONSTRAINT_DEBUG_DRAW_SIZE = 100.f;

extern void btInitCustomMaterialCombinerCallback();
DLLNETWORK PhysEnv *g_simEnvironment = nullptr;

void PhysEnv::SimulationCallback(btDynamicsWorld *world,btScalar timeStep)
{
	void *userData = world->getWorldUserInfo();
	PhysEnv *env = static_cast<PhysEnv*>(userData);
	env->SimulationCallback(timeStep);
}

// Update physics info for character controllers
static void update_physics_contact_controller_info(Game *game,int idx,const btCollisionObject *o,const btCollisionObject *oOther,btPersistentManifold *contactManifold)
{
	auto *col = static_cast<PhysCollisionObject*>(o->getUserPointer());
	if(col == nullptr)
		return;
	auto *phys = static_cast<PhysObj*>(col->userData);
	auto *colOther = static_cast<PhysCollisionObject*>(oOther->getUserPointer());
	if(phys == nullptr || phys->IsController() == false || (colOther != nullptr && colOther->IsTrigger() == true))
		return;
	auto *shape = o->getCollisionShape();
	auto numContacts = contactManifold->getNumContacts();
	for(auto i=decltype(numContacts){0};i<numContacts;++i)
	{
		auto &contactPoint = contactManifold->getContactPoint(i);
		auto &localPoint = (idx == 0) ? contactPoint.m_localPointA : contactPoint.m_localPointB;
		auto y = localPoint.y();

		// If the contact point is straight below the controller, assume this means the controller is standing on solid ground!
		const auto BOX_SHAPE_MARGIN_EPSILON = 2.0 *PhysEnv::WORLD_SCALE;
		auto bValidCandidate = false;
		switch(shape->getShapeType())
		{
			case CAPSULE_SHAPE_PROXYTYPE: // Check if contact point is touching lower half-sphere of capsule shape
				bValidCandidate = y < static_cast<const btCapsuleShape*>(shape)->getHalfHeight();
				break;
			case BOX_SHAPE_PROXYTYPE: // Check if contact point is touching bottom of box shape
				bValidCandidate = y < static_cast<const btBoxShape*>(shape)->getHalfExtentsWithoutMargin().y() +BOX_SHAPE_MARGIN_EPSILON;
		}
		if(bValidCandidate == false)
			continue;
		// This will only be applied if the contact point is a better candidate than the controller's previous candidate (for this tick)!
		static_cast<ControllerPhysObj*>(phys)->SetGroundContactPoint(contactPoint,idx,o,oOther);
	}
}

void PhysEnv::SimulationCallback(double)
{
	auto t = std::chrono::high_resolution_clock::now();
	btDispatcher *dispatcher = m_btWorld->getDispatcher();
	int numManifolds = dispatcher->getNumManifolds();
	auto *nw = GetNetworkState();
	auto *game = nw->GetGameState();
	auto bClient = game->IsClient();
	for(int i=0;i<numManifolds;i++)
	{
		btPersistentManifold *contactManifold =  dispatcher->getManifoldByIndexInternal(i);
		int numContacts = contactManifold->getNumContacts();
		if(numContacts > 0)
		{
			// Update character ground contact point
			auto *o0 = contactManifold->getBody0();
			auto *o1 = contactManifold->getBody1();
			update_physics_contact_controller_info(game,0,o0,o1,contactManifold);
			update_physics_contact_controller_info(game,1,o1,o0,contactManifold);
			//

			// Physics Sound
			if(bClient == true)
			{
				auto *obj = static_cast<PhysCollisionObject*>(o1->getUserPointer());
				if(obj != nullptr)
				{
					auto *surface = game->GetSurfaceMaterial(obj->GetSurfaceMaterial());
					if(surface != nullptr)
					{
						for(auto i=0;i<numContacts;i++)
						{
							auto &pt = contactManifold->getContactPoint(0);
							if(pt.getDistance() <= 1.f *PhysEnv::WORLD_SCALE)
							{
								auto impulse = pt.getAppliedImpulse(); // Musn't be scaled by world scale!
								const auto softImpactThreshold = 100.f;
								const auto hardImpactThreshold = 400.f;
								if(impulse >= softImpactThreshold)
								{
									auto bHardImpact = (impulse >= hardImpactThreshold) ? true : false;
									std::string sndImpact = (bHardImpact == false) ? surface->GetSoftImpactSound() : surface->GetHardImpactSound();
									if(sndImpact.empty())
									{
										surface = game->GetSurfaceMaterial(0);
										sndImpact = (bHardImpact == false) ? surface->GetSoftImpactSound() : surface->GetHardImpactSound();
									}
									auto snd = nw->CreateSound(sndImpact,ALSoundType::Effect | ALSoundType::Physics,ALCreateFlags::Mono);
									if(snd != nullptr)
									{
										auto pos = pt.getPositionWorldOnB() /PhysEnv::WORLD_SCALE;
										snd->SetPosition(Vector3(pos.x(),pos.y(),pos.z()));
										if(impulse < hardImpactThreshold)
											snd->SetGain(CFloat(0.25f +(impulse -softImpactThreshold) /(hardImpactThreshold -softImpactThreshold) *0.5f));
										else
											snd->SetGain(CFloat(0.75f +(impulse -hardImpactThreshold) /hardImpactThreshold *0.25f));
										snd->Play();
									}
									break;
								}
							}
						}
					}
				}
			}
			//

			PhysCollisionObject *colA = static_cast<PhysCollisionObject*>(o0->getUserPointer());
			PhysObj *physA = (colA != nullptr) ? static_cast<PhysObj*>(colA->userData) : nullptr;
			auto *entA = (physA != nullptr) ? &physA->GetOwner()->GetEntity() : nullptr;
			auto pPhysComponentA = (entA != nullptr) ? entA->GetPhysicsComponent() : util::WeakHandle<pragma::BasePhysicsComponent>{};

			PhysCollisionObject *colB = static_cast<PhysCollisionObject*>(o1->getUserPointer());
			PhysObj *physB = (colB != nullptr) ? static_cast<PhysObj*>(colB->userData) : nullptr;
			auto *entB = (physB != nullptr) ? &physB->GetOwner()->GetEntity() : nullptr;
			auto pPhysComponentB = (entB != nullptr) ? entB->GetPhysicsComponent() : util::WeakHandle<pragma::BasePhysicsComponent>{};
			if(pPhysComponentA.valid() && pPhysComponentB.valid())
			{
				bool bCallbackA = pPhysComponentA->GetCollisionCallbacksEnabled();
				bool bCallbackB = pPhysComponentB->GetCollisionCallbacksEnabled();
				if(bCallbackA == true || bCallbackB == true)
				{
					auto *touchComponentA = static_cast<pragma::BaseTouchComponent*>(entA->FindComponent("touch").get());
					auto *touchComponentB = static_cast<pragma::BaseTouchComponent*>(entB->FindComponent("touch").get());
					if(bCallbackA == true && touchComponentA != nullptr)
						touchComponentA->Touch(entB,physB,colA,colB);
					if(bCallbackB == true && touchComponentB != nullptr)
						touchComponentB->Touch(entA,physA,colB,colA);
					bool bReportA = pPhysComponentA->GetCollisionContactReportEnabled();
					bool bReportB = pPhysComponentB->GetCollisionContactReportEnabled();
					if(bReportA == true || bReportB == true)
					{
						for(int j=0;j<numContacts;j++)
						{
							btManifoldPoint &pt = contactManifold->getContactPoint(j);
							if(pt.getDistance() < 0.f)
							{
								const btVector3 &ptA = pt.getPositionWorldOnA();
								const btVector3 &ptB = pt.getPositionWorldOnB();
								const btVector3 &normalOnB = pt.m_normalWorldOnB;

								if(bReportA == true && touchComponentA != nullptr)
								{
									PhysContact contact {};
									contact.entA = entA;
									contact.entB = entB;
									contact.physA = physA;
									contact.physB = physB;
									contact.objA = colA;
									contact.objB = colB;
									contact.posA = Vector3(ptA.x(),ptA.y(),ptA.z()) /CFloat(PhysEnv::WORLD_SCALE);
									contact.posB = Vector3(ptB.x(),ptB.y(),ptB.z()) /CFloat(PhysEnv::WORLD_SCALE);
									contact.hitNormal = Vector3(normalOnB.x(),normalOnB.y(),normalOnB.z());
									touchComponentA->Contact(contact);
								}
								if(bReportB == true && touchComponentB != nullptr)
								{
									PhysContact contact {};
									contact.entA = entB;
									contact.entB = entA;
									contact.physA = physB;
									contact.physB = physA;
									contact.objA = colB;
									contact.objB = colA;
									contact.posA = Vector3(ptB.x(),ptB.y(),ptB.z()) /CFloat(PhysEnv::WORLD_SCALE);
									contact.posB = Vector3(ptA.x(),ptA.y(),ptA.z()) /CFloat(PhysEnv::WORLD_SCALE);
									contact.hitNormal = Vector3(normalOnB.x(),normalOnB.y(),normalOnB.z());
									touchComponentB->Contact(contact);
								}
							}
						}
					}
				}
			}
		}
	}
}

class PhysBulletWorld
	: public btWorldType
{
public:
	using btWorldType::btWorldType;
	virtual void updateAabbs() override
	{
		BT_PROFILE("updateAabbs");

		btTransform predictedTrans;
		for ( int i=0;i<m_collisionObjects.size();i++)
		{
			btCollisionObject* colObj = m_collisionObjects[i];
			btAssert(colObj->getWorldArrayIndex() == i);

			//only update aabb of active objects
			if (m_forceUpdateAllAabbs || colObj->isActive())
			{
				updateSingleAabb(colObj);
			}
			else
			{
				auto *c = static_cast<PhysCollisionObject*>(colObj->getUserPointer());
				if(c != nullptr && c->ShouldUpdateAABB())
				{
					updateSingleAabb(colObj);
					c->ResetUpdateAABBFlag();
				}
			}
		}
	};
};

PhysEnv::PhysEnv(NetworkState *state)
	: m_nwState(state)
{
	Con::cout<<"Initializing physics environment..."<<Con::endl;
	btInitCustomMaterialCombinerCallback();

	if(std::is_same<btWorldType,btSoftRigidDynamicsWorld>::value == true)
		m_btCollisionConfiguration = std::make_unique<btSoftBodyRigidBodyCollisionConfiguration>();
	else
		m_btCollisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	m_btDispatcher = std::make_unique<btCollisionDispatcher>(m_btCollisionConfiguration.get());
	m_btOverlappingPairCache = std::make_unique<btDbvtBroadphase>();
	m_btGhostPairCallback = std::make_unique<btGhostPairCallback>();
	m_btOverlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(m_btGhostPairCallback.get());
	m_overlapFilterCallback = std::make_unique<PhysOverlapFilterCallback>();
	m_btOverlappingPairCache->getOverlappingPairCache()->setOverlapFilterCallback(m_overlapFilterCallback.get());
	m_btSolver = std::make_unique<btSequentialImpulseConstraintSolver>();
	m_softBodySolver = std::unique_ptr<btSoftBodySolver>(new btDefaultSoftBodySolver);

	m_btWorld = std::make_unique<PhysBulletWorld>(m_btDispatcher.get(),m_btOverlappingPairCache.get(),m_btSolver.get(),m_btCollisionConfiguration.get(),m_softBodySolver.get());
	m_btWorld->setGravity(btVector3(0.f,0.f,0.f));
	m_btWorld->setInternalTickCallback(&PhysEnv::SimulationCallback,this);
	m_btWorld->setForceUpdateAllAabbs(false);

	m_softBodyWorldInfo = std::make_unique<btSoftBodyWorldInfo>();
	m_softBodyWorldInfo->m_broadphase = m_btOverlappingPairCache.get();
	m_softBodyWorldInfo->m_dispatcher = m_btDispatcher.get();
	m_softBodyWorldInfo->m_gravity = m_btWorld->getGravity();
	m_softBodyWorldInfo->m_sparsesdf.Initialize();

	m_buoyancySim = std::make_shared<PhysWaterBuoyancySimulator>();

	for(auto i=0;i<EVENT_COUNT;i++)
		m_callbacks.insert(std::unordered_map<size_t,std::vector<CallbackHandle>>::value_type(i,std::vector<CallbackHandle>()));
}
PhysEnv::~PhysEnv()
{
	ClearConstraints();
	ClearCollisionObjects();
	ClearControllers();
}
btSoftBodySolver &PhysEnv::GetSoftBodySolver() {return *m_softBodySolver;}
const btSoftBodySolver &PhysEnv::GetSoftBodySolver() const {return const_cast<PhysEnv*>(this)->GetSoftBodySolver();}
NetworkState *PhysEnv::GetNetworkState() const {return m_nwState;}
double PhysEnv::GetTimeScale() const
{
	if(m_nwState == nullptr)
		return 0.0;
	auto *game = m_nwState->GetGameState();
	if(game == nullptr)
		return 0.0;
	return game->GetTimeScale();
}

void PhysEnv::AddEventCallback(int eventid,const CallbackHandle &hCb)
{
	if(eventid < 0 || eventid >= EVENT_COUNT)
		return;
	m_callbacks[eventid].push_back(hCb);
}
void PhysEnv::AddConstraint(PhysConstraint *constraint)
{
	m_constraints.push_back(constraint);
	CallCallbacks<PhysConstraint>(EVENT_CONSTRAINT_CREATE,constraint);
}
void PhysEnv::AddCollisionObject(PhysCollisionObject *obj)
{
	m_collisionObjects.push_back(obj);
	CallCallbacks<PhysCollisionObject>(EVENT_COLLISION_OBJECT_CREATE,obj);
}
void PhysEnv::AddSoftBody(PhysSoftBody *obj)
{
	m_collisionObjects.push_back(obj);
	CallCallbacks<PhysCollisionObject>(EVENT_COLLISION_OBJECT_CREATE,obj);
}
void PhysEnv::AddController(PhysController *controller)
{
	m_controllers.push_back(controller);
	CallCallbacks<PhysController>(EVENT_CONTROLLER_CREATE,controller);
}
void PhysEnv::RemoveConstraint(PhysConstraint *constraint)
{
	for(unsigned int i=0;i<m_constraints.size();i++)
	{
		if(m_constraints[i] == constraint)
		{
			CallCallbacks<PhysConstraint>(EVENT_CONSTRAINT_REMOVE,constraint);
			m_btWorld->removeConstraint(constraint->GetConstraint());
			m_constraints.erase(m_constraints.begin() +i);
			break;
		}
	}
}
void PhysEnv::RemoveCollisionObject(PhysCollisionObject *obj)
{
	for(unsigned int i=0;i<m_collisionObjects.size();i++)
	{
		if(m_collisionObjects[i] == obj)
		{
			CallCallbacks<PhysCollisionObject>(EVENT_COLLISION_OBJECT_REMOVE,obj);
			obj->RemoveWorldObject();
			m_collisionObjects.erase(m_collisionObjects.begin() +i);
			break;
		}
	}
}
void PhysEnv::RemoveController(PhysController *controller)
{
	for(unsigned int i=0;i<m_controllers.size();i++)
	{
		if(m_controllers[i] == controller)
		{
			CallCallbacks<PhysController>(EVENT_CONTROLLER_REMOVE,controller);
			m_btWorld->removeCharacter(controller->GetCharacterController());
			m_controllers.erase(m_controllers.begin() +i);
			break;
		}
	}
}
const PhysWaterBuoyancySimulator &PhysEnv::GetWaterBuoyancySimulator() const {return *m_buoyancySim;}
int PhysEnv::StepSimulation(float timeStep,int maxSubSteps,float fixedTimeStep)
{
	g_simEnvironment = this;
	auto r = m_btWorld->stepSimulation(timeStep,maxSubSteps,fixedTimeStep);
	g_simEnvironment = nullptr;
	return r;
}
std::shared_ptr<PhysConvexShape> PhysEnv::CreateCapsuleShape(float halfWidth,float halfHeight)
{
	auto height = halfHeight *2.f -halfWidth *2.f;
	return CreateConvexShape(new btCapsuleShape(halfWidth *PhysEnv::WORLD_SCALE,height *PhysEnv::WORLD_SCALE));
}
std::shared_ptr<PhysConvexShape> PhysEnv::CreateBoxShape(const Vector3 &halfExtents) {return CreateConvexShape(new btBoxShape(btVector3(halfExtents.x,halfExtents.y,halfExtents.z) *PhysEnv::WORLD_SCALE));}
std::shared_ptr<PhysConvexShape> PhysEnv::CreateCylinderShape(float radius,float height) {return CreateConvexShape(new btCylinderShape(btVector3(radius,height,radius) *PhysEnv::WORLD_SCALE));}
std::shared_ptr<PhysConvexShape> PhysEnv::CreateSphereShape(float radius) {return CreateConvexShape(new btSphereShape(radius *PhysEnv::WORLD_SCALE));}
std::shared_ptr<PhysCompoundShape> PhysEnv::CreateTorusShape(uint32_t subdivisions,double outerRadius,double innerRadius)
{
	// Source: http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?t=7228#p24758
	auto rshape = CreateCompoundShape();

	btVector3 forward(btScalar(0.0),btScalar(1.0),btScalar(0.0));
	btVector3 side(btScalar(outerRadius),btScalar(0.0),btScalar(0.0));

	auto gap = sqrt(2.0 *innerRadius *innerRadius -2.0 *innerRadius *innerRadius* cos((2.0 *SIMD_PI) /static_cast<double>(subdivisions)));
	auto *shape = new btCylinderShapeZ(btVector3(btScalar(innerRadius),btScalar(innerRadius),btScalar((SIMD_PI /static_cast<double>(subdivisions)) +0.5 *gap)));
	std::shared_ptr<PhysConvexShape> cylShape(new PhysConvexShape(shape));

	for(auto i=decltype(subdivisions){0};i<subdivisions;++i)
	{
		auto angle = btScalar((static_cast<double>(i) *2.0 *SIMD_PI) /static_cast<double>(subdivisions));
		auto position = side.rotate(forward,angle);
		btQuaternion q(forward,angle);

		auto pShape = std::static_pointer_cast<PhysShape>(cylShape);
		rshape->AddShape(pShape,uvec::create(position),uquat::create(q));
	}
	return rshape;
}
PhysGhostObject *PhysEnv::CreateGhostObject(std::shared_ptr<PhysShape> shape)
{
	btPairCachingGhostObject *o = new btPairCachingGhostObject;
	PhysGhostObject *ghost = new PhysGhostObject(this,o,shape);
	ghost->Initialize();
	AddCollisionObject(ghost);
	return ghost;
}
void PhysEnv::ClearConstraints()
{
	for(unsigned int i=0;i<m_constraints.size();i++)
	{
		PhysConstraint *c = m_constraints[i];
		btTypedConstraint *constraint = c->GetConstraint();
		m_btWorld->removeConstraint(constraint);
	}
	m_constraints.clear();
}
void PhysEnv::ClearCollisionObjects()
{
	for(unsigned int i=0;i<m_collisionObjects.size();i++)
	{
		PhysCollisionObject *c = m_collisionObjects[i];
		c->RemoveWorldObject();
	}
	m_collisionObjects.clear();
}
void PhysEnv::ClearControllers()
{
	for(unsigned int i=0;i<m_controllers.size();i++)
	{
		PhysController *c = m_controllers[i];
		auto *cntr = c->GetCharacterController();
		m_btWorld->removeCharacter(cntr);
	}
	m_controllers.clear();
}
std::shared_ptr<PhysConvexHullShape> PhysEnv::CreateConvexHullShape() {return std::make_shared<PhysConvexHullShape>();}
std::shared_ptr<PhysCompoundShape> PhysEnv::CreateCompoundShape() {return std::make_shared<PhysCompoundShape>();}
std::shared_ptr<PhysCompoundShape> PhysEnv::CreateCompoundShape(std::shared_ptr<PhysShape> &shape) {return std::make_shared<PhysCompoundShape>(shape);}
std::shared_ptr<PhysCompoundShape> PhysEnv::CreateCompoundShape(std::vector<std::shared_ptr<PhysShape>> &shapes) {return std::make_shared<PhysCompoundShape>(shapes);}
std::shared_ptr<PhysShape> PhysEnv::CreateHeightfieldTerrainShape(int32_t width,int32_t length,btScalar maxHeight,int32_t upAxis) {return std::make_shared<PhysHeightfield>(width,length,maxHeight,upAxis);}
PhysCollisionObject *PhysEnv::CreateCollisionObject(std::shared_ptr<PhysShape> shape)
{
	auto *collisionObject = new PhysCollisionObject(this,new btCollisionObject(),shape);
	collisionObject->Initialize();
	AddCollisionObject(collisionObject);
	return collisionObject;
}
PhysRigidBody *PhysEnv::CreateRigidBody(float mass,std::shared_ptr<PhysShape> shape,const Vector3 &localInertia)
{
	PhysRigidBody *collisionObject = new PhysRigidBody(this,mass,shape,localInertia);
	collisionObject->Initialize();
	AddCollisionObject(collisionObject);
	return collisionObject;
}
static btSoftBody *createSoftBody(btSoftRigidDynamicsWorld *world,btSoftBodyWorldInfo *info,const btScalar s,
					const int numX,
					const int numY, 
					const int fixed) {
   btSoftBody* cloth=btSoftBodyHelpers::CreatePatch(*info,
                                                    btVector3(-s/2,s+1,0),
                                                    btVector3(+s/2,s+1,0),
		                                    btVector3(-s/2,s+1,+s),
		                                    btVector3(+s/2,s+1,+s),
		                                    numX,numY, 
		                                    fixed,true); 
   cloth->getCollisionShape()->setMargin(0.001f);
   cloth->generateBendingConstraints(2,cloth->appendMaterial());
   cloth->setTotalMass(10);  
   cloth->m_cfg.piterations = 5;
   cloth->m_cfg.kDP = 0.005f;
	world->addSoftBody(cloth);
	return cloth;
}
#pragma pack(push,1)
struct Vector3d
{
	Vector3d(double _x,double _y,double _z)
		: x(_x),y(_y),z(_z)
	{}
	Vector3d()=default;
	double x = 0.f;
	double y = 0.f;
	double z = 0.f;
};
#pragma pack(pop)
static btSoftBody *createSoftBody(const PhysSoftBodyInfo &sbInfo,btSoftRigidDynamicsWorld *world,btSoftBodyWorldInfo *info,float mass,std::vector<Vector3d> &verts,std::vector<int32_t> indices) {
	if(mass == 0.f)
	{
		Con::cwar<<"WARNING: Attempted to create soft-body object with mass of 0! Using mass of 1 instead..."<<Con::endl;
		mass = 1.f;
	}
	auto *cloth = btSoftBodyHelpers::CreateFromTriMesh(*info,reinterpret_cast<btScalar*>(verts.data()),indices.data(),indices.size() /3,true);

	auto *pm = cloth->appendMaterial();
	auto it = sbInfo.materialStiffnessCoefficient.find(0u);
	if(it != sbInfo.materialStiffnessCoefficient.end())
	{
		pm->m_kLST = it->second.linear;
		pm->m_kAST = it->second.angular;
		pm->m_kVST = it->second.volume;
	}
	pm->m_flags &= ~btSoftBody::fMaterial::DebugDraw;

	cloth->m_cfg.kAHR = sbInfo.anchorsHardness;
	cloth->m_cfg.kCHR = sbInfo.rigidContactsHardness;
	cloth->m_cfg.kDF = sbInfo.dynamicFrictionCoefficient;
	cloth->m_cfg.kDG = sbInfo.dragCoefficient;
	cloth->m_cfg.kDP = sbInfo.dampingCoefficient;
	cloth->m_cfg.kKHR = sbInfo.kineticContactsHardness;
	cloth->m_cfg.kLF = sbInfo.liftCoefficient;
	cloth->m_cfg.kMT = sbInfo.poseMatchingCoefficient;
	cloth->m_cfg.kPR = sbInfo.pressureCoefficient;
	cloth->m_cfg.kSHR = sbInfo.softContactsHardness;
	cloth->m_cfg.kSKHR_CL = sbInfo.softVsKineticHardness;
	cloth->m_cfg.kSK_SPLT_CL = sbInfo.softVsRigidImpulseSplitK;
	cloth->m_cfg.kSRHR_CL = sbInfo.softVsRigidHardness;
	cloth->m_cfg.kSR_SPLT_CL = sbInfo.softVsRigidImpulseSplitR;
	cloth->m_cfg.kSSHR_CL = sbInfo.softVsSoftHardness;
	cloth->m_cfg.kSS_SPLT_CL = sbInfo.softVsRigidImpulseSplitS;
	cloth->m_cfg.kVC = sbInfo.volumeConversationCoefficient;
	cloth->m_cfg.kVCF = sbInfo.velocitiesCorrectionFactor;

	if(sbInfo.bendingConstraintsDistance > 0.f)
		cloth->generateBendingConstraints(sbInfo.bendingConstraintsDistance,pm);

	//cloth->m_cfg.collisions = btSoftBody::fCollision::CL_RS;
	cloth->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
	// cloth->m_cfg.collisions |= btSoftBody::fCollision::VF_SS | btSoftBody::fCollision::CL_RS | btSoftBody::fCollision::CL_SS;	

/*
		RVSmask	=	0x000f,	///Rigid versus soft mask
		SDF_RS	=	0x0001,	///SDF based rigid vs soft
		CL_RS	=	0x0002, ///Cluster vs convex rigid vs soft

		SVSmask	=	0x0030,	///Rigid versus soft mask		
		VF_SS	=	0x0010,	///Vertex vs face soft vs soft handling
		CL_SS	=	0x0020, ///Cluster vs cluster soft vs soft handling
		CL_SELF =	0x0040, ///Cluster soft body self collision
		Default	=	SDF_RS,
		END
			*/

   /*cloth->getCollisionShape()->setMargin(0.001f);//0.005f);
   cloth->setTotalMass(mass);  
   cloth->m_cfg.piterations = 5;
   cloth->generateClusters(k,maxiterations);*/
  // cloth->
   //btSoftBody::eAeroModel::F_OneSided

	if(sbInfo.clusterCount > 0)
		cloth->generateClusters(sbInfo.clusterCount,sbInfo.maxClusterIterations);
	cloth->setPose(true,true);

	world->addSoftBody(cloth);
	return cloth;
}
PhysSoftBody *PhysEnv::CreateSoftBody(const PhysSoftBodyInfo &info,float mass,btAlignedObjectArray<btVector3> &vtx,const std::vector<uint16_t> &indices,std::vector<uint16_t> &indexTranslations)
{
	{
		std::vector<Vector3d> sbVerts;
		sbVerts.reserve(vtx.size());
		indexTranslations.resize(vtx.size());
		for(auto i=decltype(vtx.size()){0};i<vtx.size();++i)
		{
			auto &v = vtx.at(i);
			auto it = std::find_if(sbVerts.begin(),sbVerts.end(),[&v](const Vector3d &other) {
				const auto EPSILON = 0.01f;//1.f;
				return umath::abs(other.x -v.x()) /WORLD_SCALE < EPSILON && umath::abs(other.y -v.y()) /WORLD_SCALE < EPSILON && umath::abs(other.z -v.z()) /WORLD_SCALE < EPSILON;
			});
			if(it == sbVerts.end())
			{
				sbVerts.push_back({v.x(),v.y(),v.z()});
				it = sbVerts.end() -1;
			}
			indexTranslations.at(i) = it -sbVerts.begin();
		}
		std::vector<int32_t> sbIndices;
		/*for(auto i=decltype(vtx.size()){0};i<vtx.size();++i)
		{
			auto &v = vtx.at(i);
			sbVerts.push_back({v.x(),v.y(),v.z()});
			//sbVerts.push_back(vtx.at(i).x());
			//sbVerts.push_back(vtx.at(i).y());
			//sbVerts.push_back(vtx.at(i).z());
		}*/
	//	for(auto idx : indices)
		//	sbIndices.push_back(idx);
		//sbIndices.reserve(indices.size());
		for(auto idx : indices)
			sbIndices.push_back(indexTranslations.at(idx));


		/*std::vector<btScalar> vts;
		for(auto i=decltype(vtx.size()){0};i<vtx.size();++i)
		{
			auto &v = vtx.at(i);
			vts.push_back(v.x());
			vts.push_back(v.y());
			vts.push_back(v.z());
		}
		std::vector<int32_t> id;
		for(auto idx : indices)
		{
			id.push_back(idx);
		}*/
		auto *body = createSoftBody(info,GetWorld(),GetBtSoftBodyWorldInfo(),mass,sbVerts,sbIndices);
		//auto *body = createSoftBody(GetWorld(),GetBtSoftBodyWorldInfo(),4.f,64,64,1);
		body->activate(true);

		auto *btShape = body->getCollisionShape();
		auto shape = std::shared_ptr<PhysShape>(new PhysShape(btShape,false));
		auto *softBody = new PhysSoftBody(this,body,shape,indexTranslations);
		softBody->Initialize();
		AddSoftBody(softBody);
		return softBody;
		/*c_game->AddCallback("Tick",FunctionCallback<void>::Create([]() {
			softBody->activate(true);
			//m_dynamicsWorld->stepSimulation(c_game->DeltaTickTime(),1,c_game->DeltaTickTime());
		}));*/
	}
	{
		/*auto s = 256 *PhysEnv::WORLD_SCALE;
		auto numX = 64;
		auto numY = 64;
		auto fixed = 1 +2;
		   btSoftBody* cloth=btSoftBodyHelpers::CreatePatch(*m_softBodyWorldInfo,
															btVector3(-s/2,s+1,0),
															btVector3(+s/2,s+1,0),
													btVector3(-s/2,s+1,+s),
													btVector3(+s/2,s+1,+s),
													numX,numY, 
													fixed,true); 
		   cloth->getCollisionShape()->setMargin(0.001f);
		   cloth->generateBendingConstraints(2,cloth->appendMaterial());
		   cloth->setTotalMass(10);  
		   cloth->m_cfg.piterations = 5;
		   cloth->m_cfg.kDP = 0.005f;
		   //m_btWorld->addSoftBody(cloth);
			auto shape = std::shared_ptr<PhysShape>(new PhysShape(cloth->getCollisionShape(),false));
			auto *softBody = new PhysSoftBody(this,cloth,shape);
			softBody->Initialize();
			AddSoftBody(softBody);
		   return softBody;*/
	}
	/*{
		std::vector<int> triVertexIndices {};
		for(auto idx : indices)
			triVertexIndices.push_back(idx);
		std::vector<float> triVertexCoords {};
		for(auto i=decltype(vtx.size()){0};i<vtx.size();++i)
		{
			auto &v = vtx.at(i);
			triVertexCoords.push_back(v.x());
			triVertexCoords.push_back(v.y());
			triVertexCoords.push_back(v.z());
		}
		auto numTriangles = triVertexIndices.size() / 3;
		auto numVertices = triVertexCoords.size() / 3;
		auto triVertCoords = new btScalar[triVertexCoords.size()];
		auto triVertIndices = new int[triVertexIndices.size()];

		for(int i = 0; i < triVertexCoords.size(); i ++)
		{
			triVertCoords[i] = static_cast<btScalar>(triVertexCoords[i]);
   

		}

   
		for(int i = 0; i < triVertexIndices.size(); i ++)
		{
			triVertIndices[i] = triVertexIndices[i];
		}
		//this->m_body.reset( );
		auto *body = btSoftBodyHelpers::CreateFromTriMesh(*m_softBodyWorldInfo, triVertCoords, triVertIndices, numTriangles);
   
		btSoftBody::Material*   pm=body->appendMaterial();
		pm->m_kLST            =   0.5;
		pm->m_flags            -=   btSoftBody::fMaterial::DebugDraw;
		body->generateBendingConstraints(2,pm);
		body->m_cfg.piterations   =   3;
		body->m_cfg.kDF         =   0.5;
		body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;	
		body->scale(btVector3(1,1,1));   
		body->setTotalMass(100, true);
		body->randomizeConstraints();
   body->getCollisionShape()->setMargin(0.1f);

		auto *btShape = body->getCollisionShape();
		auto shape = std::shared_ptr<PhysShape>(new PhysShape(btShape,false));
		auto *softBody = new PhysSoftBody(this,body,shape);
		softBody->Initialize();
		AddSoftBody(softBody);
		return softBody;
	}
	auto *psb = new btSoftBody(m_softBodyWorldInfo.get(),vtx.size(),&vtx[0],0);
	uint32_t i = 0;
	uint32_t ni = 0;
	auto ntriangles = indices.size() /3;
	auto &triangles = indices;
	auto maxidx = vtx.size();
	btAlignedObjectArray<bool>		chks;
	chks.resize(maxidx*maxidx,false);
	for( i=0,ni=ntriangles*3;i<ni;i+=3)
	{
		const int idx[]={triangles[i],triangles[i+1],triangles[i+2]};
#define IDX(_x_,_y_) ((_y_)*maxidx+(_x_))
		for(int j=2,k=0;k<3;j=k++)
		{
			if(!chks[IDX(idx[j],idx[k])])
			{
				chks[IDX(idx[j],idx[k])]=true;
				chks[IDX(idx[k],idx[j])]=true;
				psb->appendLink(idx[j],idx[k]);
			}
		}
#undef IDX
		psb->appendFace(idx[0],idx[1],idx[2]);
	}*/
		/*std::vector<int32_t> ind {};
		ind.reserve(indices.size());
		for(auto idx : indices)
			ind.push_back(idx);
		auto *psb = btSoftBodyHelpers::CreateFromTriMesh(*m_softBodyWorldInfo,reinterpret_cast<btScalar*>(&vtx[0]),ind.data(),ind.size() /3);*/
		/*this->numTriangles = triVertexIndices.size() / 3;
		this->numVertices = triVertexCoords.size() / 3;
		this->triVertCoords = new btScalar[triVertexCoords.size()];
		this->triVertIndices = new int[triVertexIndices.size()];

		for(int i = 0; i < triVertexCoords.size(); i ++)
		{
		this->triVertCoords[i] = static_cast<btScalar>(triVertexCoords[i]);
   

		}

   
		for(int i = 0; i < triVertexIndices.size(); i ++)
		{
		this->triVertIndices[i] = triVertexIndices[i];
		}
		this->m_body.reset( btSoftBodyHelpers::CreateFromTriMesh(worldInfo, this->triVertCoords, this->triVertIndices, this->numTriangles));*/

	/*auto *pm = psb->appendMaterial(); // TODO
	pm->m_kLST = 0.55;
	pm->m_kAST = 0.2;
	pm->m_kVST = 0.55;*/
	/*for(auto i=decltype(indices.size()){0};i<indices.size();i+=3)
	{
		psb->appendLink(indices[i +2],indices[i],pm,true);
		psb->appendLink(indices[i],indices[i +1],pm,true);
		psb->appendLink(indices[i +1],indices[i +2],pm,true);

		psb->appendFace(indices[i],indices[i +1],indices[i +2],pm);
	}*/
   
	/*psb->generateBendingConstraints(2,pm);
	psb->m_cfg.piterations = 2;
	psb->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;// | btSoftBody::fCollision::CL_SS;

	psb->generateBendingConstraints(2,pm);
	psb->m_cfg.piterations = 3;
	psb->m_cfg.kDF = 0.5;
	psb->scale(btVector3(1,1,1));   
	psb->setTotalMass(100,true);
//	psb->randomizeConstraints();

	btTransform trans;
	trans.setIdentity();
	psb->setWorldTransform(trans);
//	psb->setTotalMass(1.0,true);
//	psb->generateClusters(64);

	auto *btShape = psb->getCollisionShape();
	auto shape = std::shared_ptr<PhysShape>(new PhysShape(btShape,false));
	auto *softBody = new PhysSoftBody(this,psb,shape);
	softBody->Initialize();
	AddSoftBody(softBody);
	return softBody;*/
}
PhysSoftBody *PhysEnv::CreateSoftBody(const PhysSoftBodyInfo &info,float mass,const std::vector<Vector3> &verts,const std::vector<uint16_t> &indices,std::vector<uint16_t> &indexTranslations)
{
	btAlignedObjectArray<btVector3> vtx;
	vtx.reserve(static_cast<int32_t>(verts.size()));
	for(auto &t : verts)
		vtx.push_back({t.x *PhysEnv::WORLD_SCALE,t.y *PhysEnv::WORLD_SCALE,t.z *PhysEnv::WORLD_SCALE});
	return CreateSoftBody(info,mass,vtx,indices,indexTranslations);
}
void PhysEnv::AddAction(btActionInterface *action) {m_btWorld->addAction(action);}
btWorldType *PhysEnv::GetWorld() {return m_btWorld.get();}
btDefaultCollisionConfiguration *PhysEnv::GetBtCollisionConfiguration() {return m_btCollisionConfiguration.get();}
btCollisionDispatcher *PhysEnv::GetBtCollisionDispatcher() {return m_btDispatcher.get();}
btBroadphaseInterface *PhysEnv::GetBtOverlappingPairCache() {return m_btOverlappingPairCache.get();}
btSequentialImpulseConstraintSolver *PhysEnv::GetBtConstraintSolver() {return m_btSolver.get();}
btSoftBodyWorldInfo *PhysEnv::GetBtSoftBodyWorldInfo() {return m_softBodyWorldInfo.get();}

std::vector<PhysConstraint*> &PhysEnv::GetConstraints() {return m_constraints;}
std::vector<PhysCollisionObject*> &PhysEnv::GetCollisionObjects() {return m_collisionObjects;}
std::vector<PhysController*> &PhysEnv::GetControllers() {return m_controllers;}

PhysFixedConstraint *PhysEnv::CreateFixedConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat &rotA,PhysRigidBody *b,const Vector3 &pivotB,const Quat &rotB)
{
	btRigidBody *bodyA = a->GetRigidBody();
	btRigidBody *bodyB = b->GetRigidBody();

	btTransform transformA;
	transformA.setIdentity();
	transformA.setOrigin(btVector3(pivotA.x,pivotA.y,pivotA.z) *WORLD_SCALE);
	transformA.setRotation(btQuaternion(rotA.x,rotA.y,rotA.z,rotA.w));

	btTransform transformB;
	transformB.setIdentity();
	transformB.setOrigin(btVector3(pivotB.x,pivotB.y,pivotB.z) *WORLD_SCALE);
	transformB.setRotation(btQuaternion(rotB.x,rotB.y,rotB.z,rotB.w));
	return AddFixedConstraint(new btFixedConstraint(*bodyA,*bodyB,transformA,transformB));
}
PhysBallSocket *PhysEnv::CreateBallSocketConstraint(PhysRigidBody *a,const Vector3 &pivotA,PhysRigidBody *b,const Vector3 &pivotB)
{
	btRigidBody *bodyA = a->GetRigidBody();
	btRigidBody *bodyB = b->GetRigidBody();
	return AddBallSocketConstraint(new btPoint2PointConstraint(*bodyA,*bodyB,btVector3(pivotA.x,pivotA.y,pivotA.z) *WORLD_SCALE,btVector3(pivotB.x,pivotB.y,pivotB.z) *WORLD_SCALE));
}
PhysHinge *PhysEnv::CreateHingeConstraint(PhysRigidBody *a,const Vector3 &pivotA,PhysRigidBody *b,const Vector3 &pivotB,const Vector3 &axis)
{
	btRigidBody *bodyA = a->GetRigidBody();
	btRigidBody *bodyB = b->GetRigidBody();
	btVector3 btAxis(axis.x,axis.y,axis.z);
	return AddHingeConstraint(new btHingeConstraint(*bodyA,*bodyB,btVector3(pivotA.x,pivotA.y,pivotA.z) *WORLD_SCALE,btVector3(pivotB.x,pivotB.y,pivotB.z) *WORLD_SCALE,btAxis,btAxis));
}
PhysSlider *PhysEnv::CreateSliderConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat&,PhysRigidBody *b,const Vector3 &pivotB,const Quat&)
{
	btRigidBody *bodyA = a->GetRigidBody();
	btRigidBody *bodyB = b->GetRigidBody();
	btTransform btTransformA;
	btTransformA.setIdentity();
	btTransformA.setOrigin(btVector3(pivotA.x,pivotA.y,pivotA.z) *WORLD_SCALE);
//	btTransformA.setRotation(btQuaternion(0,0,0,1));
//	btTransformA.getBasis().setEulerYPR(pivotB.x,pivotB.y,pivotB.z);

	//btTransformA.setOrigin(btVector3(pivotA.x,pivotA.y,pivotA.z) *WORLD_SCALE);
	//btTransformA.setRotation(btQuaternion(rotA.x,rotA.y,rotA.z,rotA.w));
	btTransform btTransformB;
	btTransformB.setIdentity();
	btTransformB.setOrigin(btVector3(pivotB.x,pivotB.y,pivotB.z) *WORLD_SCALE);
	//btTransformB.setRotation(btQuaternion(0,0,0,1));


	//btTransformB.setOrigin(btVector3(pivotB.x,pivotB.y,pivotB.z) *WORLD_SCALE);
	//btTransformB.setRotation(btQuaternion(rotB.x,rotB.y,rotB.z,rotB.w));
	//btTransformA = btTransform::getIdentity();
	//btTransformB = btTransform::getIdentity();
	return AddSliderConstraint(new btSliderConstraint(*bodyA,*bodyB,btTransformA,btTransformB,true));
}
PhysConeTwist *PhysEnv::CreateConeTwistConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat &rotA,PhysRigidBody *b,const Vector3 &pivotB,const Quat &rotB)
{
	auto v = 1.f /sqrtf(2.f); // umath::cos(M_PI_4);
	auto rotSrc = rotA;
	auto rotOffset = Quat(v,0.f,-v,0.f);
	rotSrc = rotSrc *rotOffset;
	auto rotTgt = rotB;
	rotTgt = rotTgt *rotSrc;

	auto *bodyA = a->GetRigidBody();
	auto *bodyB = b->GetRigidBody();
	btTransform btTransformA;
	btTransformA.setIdentity();
	btTransformA.setOrigin(btVector3(pivotA.x,pivotA.y,pivotA.z) *WORLD_SCALE);
	btTransformA.setRotation(btQuaternion(rotSrc.x,rotSrc.y,rotSrc.z,rotSrc.w));
	btTransform btTransformB;
	btTransformB.setIdentity();
	btTransformB.setOrigin(btVector3(pivotB.x,pivotB.y,pivotB.z) *WORLD_SCALE);
	btTransformB.setRotation(btQuaternion(rotTgt.x,rotTgt.y,rotTgt.z,rotTgt.w));
	return AddConeTwistConstraint(new btConeTwistConstraint(*bodyA,*bodyB,btTransformA,btTransformB));
}
PhysDoF *PhysEnv::CreateDoFConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat &rotA,PhysRigidBody *b,const Vector3 &pivotB,const Quat &rotB)
{
	auto *bodyA = a->GetRigidBody();
	auto *bodyB = b->GetRigidBody();

	btTransform tA;
	tA.setIdentity();
	tA.setOrigin(btVector3(pivotA.x,pivotA.y,pivotA.z) *WORLD_SCALE);
	tA.setRotation(btQuaternion(rotA.x,rotA.y,rotA.z,rotA.w));
	btTransform tB;
	tB.setIdentity();
	tB.setOrigin(btVector3(pivotB.x,pivotB.y,pivotB.z) *WORLD_SCALE);
	tB.setRotation(btQuaternion(rotB.x,rotB.y,rotB.z,rotB.w));
	return AddDoFConstraint(new btGeneric6DofConstraint(*bodyA,*bodyB,tA,tB,true));
}
PhysDoFSpringConstraint *PhysEnv::CreateDoFSpringConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat &rotA,PhysRigidBody *b,const Vector3 &pivotB,const Quat &rotB)
{
	auto *bodyA = a->GetRigidBody();
	auto *bodyB = b->GetRigidBody();

	btTransform tA;
	tA.setIdentity();
	tA.setOrigin(btVector3(pivotA.x,pivotA.y,pivotA.z) *WORLD_SCALE);
	tA.setRotation(btQuaternion(rotA.x,rotA.y,rotA.z,rotA.w));
	btTransform tB;
	tB.setIdentity();
	tB.setOrigin(btVector3(pivotB.x,pivotB.y,pivotB.z) *WORLD_SCALE);
	tB.setRotation(btQuaternion(rotB.x,rotB.y,rotB.z,rotB.w));

	auto *c = new btGeneric6DofSpring2Constraint(*bodyA,*bodyB,tA,tB);
	c->setDbgDrawSize(PHYS_CONSTRAINT_DEBUG_DRAW_SIZE);
	auto *constraint = new PhysDoFSpringConstraint(this,c);
	AddConstraint(constraint);
	return constraint;
}

PhysFixedConstraint *PhysEnv::AddFixedConstraint(btFixedConstraint *c)
{
	c->setDbgDrawSize(PHYS_CONSTRAINT_DEBUG_DRAW_SIZE);
	auto *constraint = new PhysFixedConstraint(this,c);
	AddConstraint(constraint);
	return constraint;
}
PhysBallSocket *PhysEnv::AddBallSocketConstraint(btPoint2PointConstraint *c)
{
	c->setDbgDrawSize(PHYS_CONSTRAINT_DEBUG_DRAW_SIZE);
	auto *constraint = new PhysBallSocket(this,c);
	AddConstraint(constraint);
	return constraint;
}
PhysHinge *PhysEnv::AddHingeConstraint(btHingeConstraint *c)
{
	c->setDbgDrawSize(PHYS_CONSTRAINT_DEBUG_DRAW_SIZE);
	auto *constraint = new PhysHinge(this,c);
	AddConstraint(constraint);
	return constraint;
}
PhysSlider *PhysEnv::AddSliderConstraint(btSliderConstraint *c)
{
	c->setDbgDrawSize(PHYS_CONSTRAINT_DEBUG_DRAW_SIZE);
	auto *constraint = new PhysSlider(this,c);
	AddConstraint(constraint);
	return constraint;
}
PhysConeTwist *PhysEnv::AddConeTwistConstraint(btConeTwistConstraint *c)
{
	c->setDbgDrawSize(PHYS_CONSTRAINT_DEBUG_DRAW_SIZE);
	auto *constraint = new PhysConeTwist(this,c);
	AddConstraint(constraint);
	return constraint;
}
PhysDoF *PhysEnv::AddDoFConstraint(btGeneric6DofConstraint *c)
{
	c->setDbgDrawSize(PHYS_CONSTRAINT_DEBUG_DRAW_SIZE);
	auto *constraint = new PhysDoF(this,c);
	AddConstraint(constraint);
	return constraint;
}
PhysDoFSpringConstraint *PhysEnv::AddDoFSpringConstraint(btGeneric6DofSpring2Constraint *c)
{
	c->setDbgDrawSize(PHYS_CONSTRAINT_DEBUG_DRAW_SIZE);
	auto *constraint = new PhysDoFSpringConstraint(this,c);
	AddConstraint(constraint);
	return constraint;
}

std::shared_ptr<PhysConvexShape> PhysEnv::CreateConvexShape(btConvexShape *shape) {return std::shared_ptr<PhysConvexShape>(new PhysConvexShape(shape));}

PhysController *PhysEnv::CreateCapsuleController(float halfWidth,float halfHeight,float stepHeight)
{
	PhysTransform st;
	return CreateCapsuleController(halfWidth,halfHeight,stepHeight,st);
}
PhysController *PhysEnv::CreateCapsuleController(float halfWidth,float halfHeight,float stepHeight,const PhysTransform &startTransform)
{
	PhysController *controller = new PhysController(this,halfWidth,halfHeight,stepHeight,startTransform);
	AddController(controller);
	return controller;
}

PhysController *PhysEnv::CreateBoxController(const Vector3 &halfExtents,float stepHeight)
{
	PhysTransform st;
	return CreateBoxController(halfExtents,stepHeight,st);
}
PhysController *PhysEnv::CreateBoxController(const Vector3 &halfExtents,float stepHeight,const PhysTransform &startTransform)
{
	PhysController *controller = new PhysController(this,halfExtents,stepHeight,startTransform);
	AddController(controller);
	return controller;
}

Bool PhysEnv::RayCast(const TraceData &data,std::vector<TraceResult> *results)
{
	auto origin = data.GetSourceOrigin();
	auto btOrigin = uvec::create_bt(origin) *WORLD_SCALE;
	auto btEnd = uvec::create_bt(data.GetTargetOrigin()) *WORLD_SCALE;
#ifdef _DEBUG
	//if(uvec::cmp(origin,data.GetTargetOrigin()) == true)
	//	btEnd.setX(btEnd.getX() +1.f); // Move it slightly, so source and target aren't the same (Causes fuzzyZero assertion error in debug mode)
#endif
	auto *filter = data.GetFilter();
	auto flags = data.GetFlags();
	auto mask = data.GetCollisionFilterMask();
	auto group = data.GetCollisionFilterGroup();
	if((UInt32(flags) &UInt32(FTRACE::ALL_HITS)) == 0 || results == nullptr)
	{
		auto btResult = (filter != nullptr) ? filter->CreateClosestRayCallbackFilter(flags,group,mask,btOrigin,btEnd) : PhysClosestRayResultCallback(btOrigin,btEnd,flags,group,mask,nullptr);
		m_btWorld->rayTest(btOrigin,btEnd,btResult);
		if(results != nullptr)
		{
			results->push_back(TraceResult());
			auto &r = results->back();
			r.startPosition = origin;
			r.hit = btResult.hasHit();
			r.fraction = CFloat(btResult.m_closestHitFraction);
			if(r.hit == false)
				r.position = data.GetTargetOrigin();
			else
				r.position = uvec::create(btResult.m_hitPointWorld /WORLD_SCALE);
			r.distance = uvec::distance(r.position,origin);
			r.normal = uvec::create(btResult.m_hitNormalWorld);
		}
		if(btResult.hasHit() == true)
		{
			if(results == nullptr)
				return true;
			auto *obj = btResult.m_collisionObject;
			auto *colObj = static_cast<PhysCollisionObject*>(obj->getUserPointer());
			if(colObj != nullptr)
			{
				auto &r = results->back();
				r.collisionObj = colObj->GetHandle();
				auto *physObj = static_cast<PhysObj*>(colObj->userData);
				if(physObj != nullptr)
				{
					r.physObj = physObj->GetHandle();
					auto *ent = physObj->GetOwner();
					if(ent != nullptr)
						r.entity = &ent->GetEntity();
				}
			}
		}
		return btResult.hasHit();
	}
	else
	{
		auto btResult = (filter != nullptr) ? filter->CreateAllHitsRayCallbackFilter(flags,group,mask,btOrigin,btEnd) : PhysAllHitsRayResultCallback(btOrigin,btEnd,flags,group,mask);
		m_btWorld->rayTest(btOrigin,btEnd,btResult);
		if(btResult.hasHit() == true)
		{
			for(int i=0;i<btResult.m_collisionObjects.size();i++)
			{
				results->push_back(TraceResult());
				auto &r = results->back();
				r.startPosition = origin;
				r.hit = true;
				if(r.hit == false)
					r.position = data.GetTargetOrigin();
				else
					r.position = uvec::create(btResult.m_hitPointWorld[i] /WORLD_SCALE);
				r.distance = uvec::distance(r.position,origin);
				r.normal = uvec::create(btResult.m_hitNormalWorld[i]);
				if(i == 0)
					r.fraction = CFloat(btResult.m_closestHitFraction);
				else
				{
					auto distance = data.GetDistance();
					r.fraction = (distance != 0.f) ? r.distance /distance : 0.f;
				}
				auto *obj = btResult.m_collisionObjects[i];
				auto *colObj = static_cast<PhysCollisionObject*>(obj->getUserPointer());
				if(colObj != nullptr)
				{
					r.collisionObj = colObj->GetHandle();
					auto *physObj = static_cast<PhysObj*>(colObj->userData);
					if(physObj != nullptr)
					{
						r.physObj = physObj->GetHandle();
						auto *ent = physObj->GetOwner();
						if(ent != nullptr)
							r.entity = &ent->GetEntity();
					}
				}
			}
		}
		return btResult.hasHit();
	}
}

Bool PhysEnv::Overlap(const TraceData &data,TraceResult *result)
{
	std::vector<PhysCollisionObject*> objs;
	if(data.GetCollisionObjects(objs) == false)
		return false;
	auto *filter = data.GetFilter();
	auto flags = data.GetFlags();
	auto group = data.GetCollisionFilterGroup();
	auto mask = data.GetCollisionFilterMask();
	auto btResult = (filter != nullptr) ? filter->CreateContactCallbackFilter(flags,group,mask) : PhysContactResultCallback(flags,group,mask);
	for(auto it=objs.begin();it!=objs.end();it++)
	{
		auto *o = *it;
		auto *btObj = o->GetCollisionObject();
		auto btTransform = btObj->getWorldTransform();
		if(data.HasTarget())
		{
			auto &t = data.GetTarget();
			btTransform = t.GetTransform();
		}
		btVector3 aabbMin,aabbMax;
		btObj->getCollisionShape()->getAabb(btTransform,aabbMin,aabbMax);
		btSingleContactCallbackCustom contactCB(btObj,m_btWorld.get(),btResult);
		auto broadPhase = m_btWorld->getBroadphase();
        broadPhase->aabbTest(aabbMin,aabbMax,contactCB);
		/*m_btWorld->contactTest(btObj,btResult);*/
		if(btResult.hasHit == true)
			break;
	}
	if(result == nullptr)
		return btResult.hasHit;
	auto &r = *result;
	r.hit = btResult.hasHit;
	r.position = btResult.m_positionWorldOnA;
	auto origin = data.GetSourceOrigin();
	r.distance = uvec::distance(r.position,origin);
	if(btResult.hasHit == true)
	{
		auto *obj = btResult.m_colObj;
		auto *colObj = static_cast<PhysCollisionObject*>(obj->getUserPointer());
		if(colObj != nullptr)
		{
			r.collisionObj = colObj->GetHandle();
			auto *physObj = static_cast<PhysObj*>(colObj->userData);
			if(physObj != nullptr)
			{
				r.physObj = physObj->GetHandle();
				auto *ent = physObj->GetOwner();
				if(ent != nullptr)
					r.entity = &ent->GetEntity();
			}
		}
	}
	return btResult.hasHit;
}
Bool PhysEnv::Sweep(const TraceData &data,TraceResult *result)
{
	std::vector<btConvexShape*> shapes;
	if(data.GetShapes(shapes) == false)
		return false;
	auto *filter = data.GetFilter();
	auto flags = data.GetFlags();
	auto group = data.GetCollisionFilterGroup();
	auto mask = data.GetCollisionFilterMask();
	auto btOrigin = uvec::create_bt(data.GetSourceOrigin()) *CFloat(WORLD_SCALE);
	auto btEnd = uvec::create_bt(data.GetTargetOrigin()) *CFloat(WORLD_SCALE);
	auto &tStart = data.GetSource();
	auto &tEnd = data.GetTarget();
	auto btResult = (filter != nullptr) ? filter->CreateClosestConvexCallbackFilter(flags,group,mask,btOrigin,btEnd) : PhysClosestConvexResultCallback(btOrigin,btEnd,flags,group,mask);
	for(auto it=shapes.begin();it!=shapes.end();it++)
	{
		auto *shape = *it;
		m_btWorld->convexSweepTest(shape,tStart.GetTransform(),tEnd.GetTransform(),btResult);
		if(btResult.hasHit() == true)
			break;
	}
	if(result != nullptr)
	{
		auto &r = *result;
		r.startPosition = data.GetSourceOrigin();
		r.hit = btResult.hasHit();
		r.fraction = CFloat(btResult.m_closestHitFraction);
		if(r.hit == false)
			r.position = data.GetTargetOrigin();
		else
			r.position = uvec::create(btResult.m_hitPointWorld /WORLD_SCALE);
		r.distance = uvec::distance(r.position,tStart.GetOrigin());
		r.normal = uvec::create(btResult.m_hitNormalWorld);
		if(btResult.hasHit() == true)
		{
			auto *obj = btResult.m_hitCollisionObject;
			auto *colObj = static_cast<PhysCollisionObject*>(obj->getUserPointer());
			if(colObj != nullptr)
			{
				r.collisionObj = colObj->GetHandle();
				auto *physObj = static_cast<PhysObj*>(colObj->userData);
				if(physObj != nullptr)
				{
					r.physObj = physObj->GetHandle();
					auto *ent = physObj->GetOwner();
					if(ent != nullptr)
						r.entity = &ent->GetEntity();
				}
			}
		}
	}
	return btResult.hasHit();
}
