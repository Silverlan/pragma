#ifndef __PHYSENVIRONMENT_H__
#define __PHYSENVIRONMENT_H__

#include "pragma/networkdefinitions.h"
#include <pragma/physics/physapi.h>
#include <mathutil/glmutil.h>
#include "pragma/physics/phystransform.h"
#include <sharedutils/functioncallback.h>
#include <pragma/math/vector/wvvector3.h>
#include <vector>
#include <unordered_map>
#ifdef __linux__
#include <BulletSoftBody/btSoftBody.h>
#include <BulletSoftBody/btSoftBodySolvers.h>
#endif

#define PHYS_USE_SOFT_RIGID_DYNAMICS_WORLD 1

#if PHYS_USE_SOFT_RIGID_DYNAMICS_WORLD == 1
	using btWorldType = btSoftRigidDynamicsWorld;
#else
	using btWorldType = btDiscreteDynamicsWorld;
#endif

class PhysSoftBody;
class PhysCollisionObject;
class PhysRigidBody;
class PhysConstraint;
class PhysFixedConstraint;
class PhysBallSocket;
class PhysHinge;
class PhysSlider;
class PhysConeTwist;
class PhysDoF;
class PhysDoFSpringConstraint;
class PhysShape;
class PhysConvexShape;
class PhysController;
class PhysGhostObject;
class PhysMotionState;
class BaseEntity;
class PhysObj;
class PhysConvexHullShape;
class PhysCompoundShape;
class PhysOverlapFilterCallback;
struct TraceResult;
class TraceData;
class PhysWaterBuoyancySimulator;
struct PhysSoftBodyInfo;
enum class FTRACE;
class DLLNETWORK PhysEnv
{
public:
	friend PhysCollisionObject;
	friend PhysConstraint;
	friend PhysController;
	enum
	{
		EVENT_CONSTRAINT_CREATE,
		EVENT_COLLISION_OBJECT_CREATE,
		EVENT_CONTROLLER_CREATE,
		EVENT_CONSTRAINT_REMOVE,
		EVENT_COLLISION_OBJECT_REMOVE,
		EVENT_CONTROLLER_REMOVE,
		EVENT_COUNT
	};
public:
	static const double WORLD_SCALE;
	static const double WORLD_SCALE_SQR; // = WORLD_SCALE^2, required for torque
	static const float CCD_MOTION_THRESHOLD;
	static const float CCD_SWEPT_SPHERE_RADIUS;
protected:
	mutable NetworkState *m_nwState;
	std::unique_ptr<btWorldType> m_btWorld = nullptr;
	std::shared_ptr<PhysWaterBuoyancySimulator> m_buoyancySim;
	std::unique_ptr<btDefaultCollisionConfiguration> m_btCollisionConfiguration = nullptr;
	std::unique_ptr<btCollisionDispatcher> m_btDispatcher = nullptr;
	std::unique_ptr<btBroadphaseInterface> m_btOverlappingPairCache = nullptr;
	std::unique_ptr<PhysOverlapFilterCallback> m_overlapFilterCallback;
	std::unique_ptr<btSequentialImpulseConstraintSolver> m_btSolver = nullptr;
	std::unique_ptr<btGhostPairCallback> m_btGhostPairCallback = nullptr;
	std::unique_ptr<btSoftBodySolver> m_softBodySolver = nullptr;
	std::unique_ptr<btSoftBodyWorldInfo> m_softBodyWorldInfo;
	std::vector<PhysConstraint*> m_constraints;
	std::vector<PhysCollisionObject*> m_collisionObjects;
	std::vector<PhysController*> m_controllers;
	std::unordered_map<size_t,std::vector<CallbackHandle>> m_callbacks;
	PhysSoftBody *CreateSoftBody(const PhysSoftBodyInfo &info,float mass,btAlignedObjectArray<btVector3> &vtx,const std::vector<uint16_t> &indices,std::vector<uint16_t> &indexTranslations);

	void ClearConstraints();
	void ClearCollisionObjects();
	void ClearControllers();
	void AddAction(btActionInterface *action);

	void AddConstraint(PhysConstraint *constraint);
	void AddCollisionObject(PhysCollisionObject *obj);
	void AddSoftBody(PhysSoftBody *softBody);
	void AddController(PhysController *controller);

	void RemoveConstraint(PhysConstraint *constraint);
	void RemoveCollisionObject(PhysCollisionObject *obj);
	void RemoveSoftBody(PhysSoftBody *softBody);
	void RemoveController(PhysController *controller);

	void SimulationCallback(double timeStep);

	template<class T>
		void CallCallbacks(int eventid,T *obj);
public:
	PhysEnv(NetworkState *state);
	~PhysEnv();
	NetworkState *GetNetworkState() const;
	double GetTimeScale() const;
	void AddEventCallback(int eventid,const CallbackHandle &hCb);
	btWorldType *GetWorld();
	btDefaultCollisionConfiguration *GetBtCollisionConfiguration();
	btCollisionDispatcher *GetBtCollisionDispatcher();
	btBroadphaseInterface *GetBtOverlappingPairCache();
	btSequentialImpulseConstraintSolver *GetBtConstraintSolver();
	btSoftBodyWorldInfo *GetBtSoftBodyWorldInfo();
	std::vector<PhysConstraint*> &GetConstraints();
	std::vector<PhysCollisionObject*> &GetCollisionObjects();
	std::vector<PhysController*> &GetControllers();
	const PhysWaterBuoyancySimulator &GetWaterBuoyancySimulator() const;
	btSoftBodySolver &GetSoftBodySolver();
	const btSoftBodySolver &GetSoftBodySolver() const;

	PhysFixedConstraint *CreateFixedConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat &rotA,PhysRigidBody *b,const Vector3 &pivotB,const Quat &rotB);
	PhysBallSocket *CreateBallSocketConstraint(PhysRigidBody *a,const Vector3 &pivotA,PhysRigidBody *b,const Vector3 &pivotB);
	PhysHinge *CreateHingeConstraint(PhysRigidBody *a,const Vector3 &pivotA,PhysRigidBody *b,const Vector3 &pivotB,const Vector3 &axis);
	PhysSlider *CreateSliderConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat &rotA,PhysRigidBody *b,const Vector3 &pivotB,const Quat &rotB);
	PhysConeTwist *CreateConeTwistConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat &rotA,PhysRigidBody *b,const Vector3 &pivotB,const Quat &rotB);
	PhysDoF *CreateDoFConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat &rotA,PhysRigidBody *b,const Vector3 &pivotB,const Quat &rotB);
	PhysDoFSpringConstraint *CreateDoFSpringConstraint(PhysRigidBody *a,const Vector3 &pivotA,const Quat &rotA,PhysRigidBody *b,const Vector3 &pivotB,const Quat &rotB);

	PhysController *CreateCapsuleController(float halfWidth,float halfHeight,float stepHeight);
	PhysController *CreateCapsuleController(float halfWidth,float halfHeight,float stepHeight,const PhysTransform &startTransform);
	PhysController *CreateBoxController(const Vector3 &halfExtents,float stepHeight);
	PhysController *CreateBoxController(const Vector3 &halfExtents,float stepHeight,const PhysTransform &startTransform);
	PhysCollisionObject *CreateCollisionObject(std::shared_ptr<PhysShape> shape);
	PhysRigidBody *CreateRigidBody(float mass,std::shared_ptr<PhysShape> shape,const Vector3 &localInertia);
	PhysSoftBody *CreateSoftBody(const PhysSoftBodyInfo &info,float mass,const std::vector<Vector3> &verts,const std::vector<uint16_t> &indices,std::vector<uint16_t> &indexTranslations);

	PhysGhostObject *CreateGhostObject(std::shared_ptr<PhysShape> shape);
	std::shared_ptr<PhysConvexShape> CreateCapsuleShape(float halfWidth,float halfHeight);
	std::shared_ptr<PhysConvexShape> CreateBoxShape(const Vector3 &halfExtents);
	std::shared_ptr<PhysConvexShape> CreateCylinderShape(float radius,float height);
	std::shared_ptr<PhysCompoundShape> CreateTorusShape(uint32_t subdivisions,double outerRadius,double innerRadius);
	std::shared_ptr<PhysConvexShape> CreateSphereShape(float radius);
	std::shared_ptr<PhysConvexHullShape> CreateConvexHullShape();
	std::shared_ptr<PhysCompoundShape> CreateCompoundShape();
	std::shared_ptr<PhysCompoundShape> CreateCompoundShape(std::shared_ptr<PhysShape> &shape);
	std::shared_ptr<PhysCompoundShape> CreateCompoundShape(std::vector<std::shared_ptr<PhysShape>> &shapes);
	std::shared_ptr<PhysShape> CreateHeightfieldTerrainShape(int32_t width,int32_t length,btScalar maxHeight,int32_t upAxis);

	int StepSimulation(float timeStep,int maxSubSteps=1,float fixedTimeStep=(1.f /60.f));
	static void SimulationCallback(btDynamicsWorld *world,btScalar timeStep);

	Bool Overlap(const TraceData &data,TraceResult *result=nullptr);
	Bool RayCast(const TraceData &data,std::vector<TraceResult> *results=nullptr);
	Bool Sweep(const TraceData &data,TraceResult *result=nullptr);

	// For internal or debugging purposes only!
	PhysFixedConstraint *AddFixedConstraint(btFixedConstraint *c);
	PhysBallSocket *AddBallSocketConstraint(btPoint2PointConstraint *c);
	PhysHinge *AddHingeConstraint(btHingeConstraint *c);
	PhysSlider *AddSliderConstraint(btSliderConstraint *c);
	PhysConeTwist *AddConeTwistConstraint(btConeTwistConstraint *c);
	PhysDoF *AddDoFConstraint(btGeneric6DofConstraint *c);
	PhysDoFSpringConstraint *AddDoFSpringConstraint(btGeneric6DofSpring2Constraint *c);

	std::shared_ptr<PhysConvexShape> CreateConvexShape(btConvexShape *shape);
};

template<class T>
	void PhysEnv::CallCallbacks(int eventid,T *obj)
{
	auto itCb = m_callbacks.find(eventid);
	if(itCb == m_callbacks.end())
		return;
	auto &v = itCb->second;
	for(auto it=v.begin();it!=v.end();it++)
	{
		auto &hCallback = *it;
		hCallback(obj);
	}
}

#endif