#ifndef __PHYSENVIRONMENT_H__
#define __PHYSENVIRONMENT_H__

#include "pragma/networkdefinitions.h"
#include <pragma/physics/physapi.h>
#include <mathutil/glmutil.h>
#include "pragma/physics/transform.hpp"
#include <sharedutils/functioncallback.h>
#include <sharedutils/util_shared_handle.hpp>
#include <pragma/math/vector/wvvector3.h>
#include <vector>
#include <unordered_map>
#ifdef __linux__
#include <BulletSoftBody/btSoftBody.h>
#include <BulletSoftBody/btSoftBodySolvers.h>
#endif

class NetworkState;
class BaseEntity;
class PhysObj;
struct TraceResult;
class TraceData;
struct PhysSoftBodyInfo;
enum class RayCastFlags : uint32_t;

namespace pragma::physics
{
	class IBase;
	class IShape;
	class IConvexShape;
	class IMaterial;
	class IConstraint;
	class IFixedConstraint;
	class IBallSocketConstraint;
	class IHingeConstraint;
	class ISliderConstraint;
	class IConeTwistConstraint;
	class IDoFConstraint;
	class IDoFSpringConstraint;

	class IController;
	class ICollisionObject;
	class IRigidBody;
	class ISoftBody;
	
	class IGhostObject;
	class ICompoundShape;
	class IConvexHullShape;
	class ITriangleShape;

	class WaterBuoyancySimulator;
	class IVisualDebugger;

	using Scalar = double;
	class DLLNETWORK IEnvironment
	{
	public:
		enum class Event : uint32_t
		{
			OnConstraintCreated = 0,
			OnCollisionObjectCreated,
			OnControllerCreated,

			OnConstraintRemoved,
			OnCollisionObjectRemoved,
			OnControllerRemoved,

			Count
		};
		static constexpr float DEFAULT_CHARACTER_SLOPE_LIMIT = 45.f;
		template<class T,typename... TARGS>
			static std::shared_ptr<T> CreateSharedPtr(TARGS&& ...args);
		template<class T,typename... TARGS>
			static util::TSharedHandle<T> CreateSharedHandle(TARGS&& ...args);
		using RemainingDeltaTime = float;

		IEnvironment(NetworkState &state);
		virtual ~IEnvironment();
		NetworkState &GetNetworkState() const;
		double GetTimeScale() const;
		void AddEventCallback(Event eventid,const CallbackHandle &hCb);
		IMaterial &GetGenericMaterial() const;

		virtual float GetWorldScale() const;

		virtual IVisualDebugger *InitializeVisualDebugger()=0;
		IVisualDebugger *GetVisualDebugger() const;

		virtual util::TSharedHandle<IFixedConstraint> CreateFixedConstraint(IRigidBody &a,const Vector3 &pivotA,const Quat &rotA,IRigidBody &b,const Vector3 &pivotB,const Quat &rotB)=0;
		virtual util::TSharedHandle<IBallSocketConstraint> CreateBallSocketConstraint(IRigidBody &a,const Vector3 &pivotA,IRigidBody &b,const Vector3 &pivotB)=0;
		virtual util::TSharedHandle<IHingeConstraint> CreateHingeConstraint(IRigidBody &a,const Vector3 &pivotA,IRigidBody &b,const Vector3 &pivotB,const Vector3 &axis)=0;
		virtual util::TSharedHandle<ISliderConstraint> CreateSliderConstraint(IRigidBody &a,const Vector3 &pivotA,const Quat &rotA,IRigidBody &b,const Vector3 &pivotB,const Quat &rotB)=0;
		virtual util::TSharedHandle<IConeTwistConstraint> CreateConeTwistConstraint(IRigidBody &a,const Vector3 &pivotA,const Quat &rotA,IRigidBody &b,const Vector3 &pivotB,const Quat &rotB)=0;
		virtual util::TSharedHandle<IDoFConstraint> CreateDoFConstraint(IRigidBody &a,const Vector3 &pivotA,const Quat &rotA,IRigidBody &b,const Vector3 &pivotB,const Quat &rotB)=0;
		virtual util::TSharedHandle<IDoFSpringConstraint> CreateDoFSpringConstraint(IRigidBody &a,const Vector3 &pivotA,const Quat &rotA,IRigidBody &b,const Vector3 &pivotB,const Quat &rotB)=0;

		virtual util::TSharedHandle<IController> CreateCapsuleController(float halfWidth,float halfHeight,float stepHeight,float slopeLimitDeg=DEFAULT_CHARACTER_SLOPE_LIMIT,const Transform &startTransform={})=0;
		virtual util::TSharedHandle<IController> CreateBoxController(const Vector3 &halfExtents,float stepHeight,float slopeLimitDeg=DEFAULT_CHARACTER_SLOPE_LIMIT,const Transform &startTransform={})=0;
		virtual util::TSharedHandle<ICollisionObject> CreateCollisionObject(IShape &shape)=0;
		virtual util::TSharedHandle<IRigidBody> CreateRigidBody(float mass,IShape &shape,const Vector3 &localInertia)=0;
		virtual util::TSharedHandle<ISoftBody> CreateSoftBody(const PhysSoftBodyInfo &info,float mass,const std::vector<Vector3> &verts,const std::vector<uint16_t> &indices,std::vector<uint16_t> &indexTranslations)=0;
		virtual util::TSharedHandle<IGhostObject> CreateGhostObject(IShape &shape)=0;

		virtual std::shared_ptr<IConvexShape> CreateCapsuleShape(float halfWidth,float halfHeight,const IMaterial &mat)=0;
		virtual std::shared_ptr<IConvexShape> CreateBoxShape(const Vector3 &halfExtents,const IMaterial &mat)=0;
		virtual std::shared_ptr<IConvexShape> CreateCylinderShape(float radius,float height,const IMaterial &mat)=0;
		virtual std::shared_ptr<ICompoundShape> CreateTorusShape(uint32_t subdivisions,double outerRadius,double innerRadius,const IMaterial &mat)=0;
		virtual std::shared_ptr<IConvexShape> CreateSphereShape(float radius,const IMaterial &mat)=0;
		virtual std::shared_ptr<IConvexHullShape> CreateConvexHullShape(const IMaterial &mat)=0;
		virtual std::shared_ptr<ITriangleShape> CreateTriangleShape(const IMaterial &mat)=0;
		virtual std::shared_ptr<ICompoundShape> CreateCompoundShape()=0;
		virtual std::shared_ptr<ICompoundShape> CreateCompoundShape(IShape &shape)=0;
		virtual std::shared_ptr<ICompoundShape> CreateCompoundShape(std::vector<IShape*> &shapes)=0;
		virtual std::shared_ptr<IShape> CreateHeightfieldTerrainShape(uint32_t width,uint32_t length,Scalar maxHeight,uint32_t upAxis,const IMaterial &mat)=0;
		virtual std::shared_ptr<IMaterial> CreateMaterial(float staticFriction,float dynamicFriction,float restitution)=0;

		virtual RemainingDeltaTime StepSimulation(float timeStep,int maxSubSteps=1,float fixedTimeStep=(1.f /60.f))=0;

		virtual Bool Overlap(const TraceData &data,std::vector<TraceResult> *optOutResults=nullptr) const=0;
		virtual Bool RayCast(const TraceData &data,std::vector<TraceResult> *optOutResults=nullptr) const=0;
		virtual Bool Sweep(const TraceData &data,std::vector<TraceResult> *optOutResults=nullptr) const=0;

		const std::vector<util::TSharedHandle<IConstraint>> &GetConstraints() const;
		std::vector<util::TSharedHandle<IConstraint>> &GetConstraints();
		const std::vector<util::TSharedHandle<ICollisionObject>> &GetCollisionObjects() const;
		std::vector<util::TSharedHandle<ICollisionObject>> &GetCollisionObjects();
		const std::vector<util::TSharedHandle<IController>> &GetControllers() const;
		std::vector<util::TSharedHandle<IController>> &GetControllers();

		virtual void RemoveConstraint(IConstraint &constraint);
		virtual void RemoveCollisionObject(ICollisionObject &obj);
		virtual void RemoveController(IController &controller);

		const WaterBuoyancySimulator &GetWaterBuoyancySimulator() const;
	protected:
		friend IConstraint;
		friend ICollisionObject;
		friend IController;
		void ClearConstraints();
		void ClearCollisionObjects();
		void ClearControllers();

		std::vector<util::TSharedHandle<IConstraint>> m_constraints = {};
		std::vector<util::TSharedHandle<ICollisionObject>> m_collisionObjects = {};
		std::vector<util::TSharedHandle<IController>> m_controllers = {};

		void AddConstraint(IConstraint &constraint);
		void AddCollisionObject(ICollisionObject &obj);
		void AddController(IController &controller);

		template<class T>
			void CallCallbacks(Event eventid,T &obj);
	protected:
		std::shared_ptr<pragma::physics::IVisualDebugger> m_visualDebugger = nullptr;
	private:
		NetworkState &m_nwState;
		std::unordered_map<Event,std::vector<CallbackHandle>> m_callbacks = {};
		std::shared_ptr<WaterBuoyancySimulator> m_buoyancySim = nullptr;
		std::shared_ptr<IMaterial> m_genericMaterial = nullptr;
	};
};

template<class T,typename... TARGS>
std::shared_ptr<T> pragma::physics::IEnvironment::CreateSharedPtr(TARGS&& ...args)
{
	return std::shared_ptr<T>{
		new T{std::forward<TARGS>(args)...},[](T *o) {
			o->OnRemove();
			delete o;
		}
	};
}
template<class T,typename... TARGS>
util::TSharedHandle<T> pragma::physics::IEnvironment::CreateSharedHandle(TARGS&& ...args)
{
	auto handle = util::TSharedHandle<T>{
		new T{std::forward<TARGS>(args)...},[](T *o) {
			o->OnRemove();
			delete o;
		}
	};
	handle->SetHandle(util::shared_handle_cast<T,IBase>(handle));
	return handle;
}

template<class T>
	void pragma::physics::IEnvironment::CallCallbacks(Event eventid,T &obj)
{
	auto itCb = m_callbacks.find(eventid);
	if(itCb == m_callbacks.end())
		return;
	auto &v = itCb->second;
	for(auto it=v.begin();it!=v.end();)
	{
		auto &hCallback = *it;
		if(hCallback.IsValid() == false)
		{
			it = v.erase(it);
			continue;
		}
		hCallback.Call<void,T*>(&obj);
		++it;
	}
}

#endif