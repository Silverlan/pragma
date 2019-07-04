#ifndef __RAYTRACES_H__
#define __RAYTRACES_H__

#include <mathutil/glmutil.h>
#include <pragma/physics/physapi.h>
#include "pragma/physics/transform.hpp"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/raycallback/physraycallback.hpp"
#include "pragma/physics/raycallback/physraycallback_contact.hpp"
#include <memory>

enum class RayCastFlags : uint32_t
{
	None = 0u,
	ReportHitPosition = 1u,
	ReportHitNormal = ReportHitPosition<<1u,
	ReportHitUV = ReportHitNormal<<1u,
	ReportAllResults = ReportHitUV<<1u,
	ReportAnyResult = ReportAllResults<<1u,
	ReportBackFaceHits = ReportAnyResult<<1u,
	Precise = ReportBackFaceHits<<1u,

	IgnoreDynamic = Precise<<1u,
	IgnoreStatic = IgnoreDynamic<<1u,
	InvertFilter = IgnoreStatic<<1u,

	Default = ReportHitPosition | ReportHitNormal | ReportHitUV
};
REGISTER_BASIC_BITWISE_OPERATORS(RayCastFlags);

enum class RayCastHitType : uint8_t
{
	None = 0,
	Touch,
	Block
};

// #define ENABLE_DEPRECATED_PHYSICS
#ifdef ENABLE_DEPRECATED_PHYSICS
class PhysEnv;
class BaseEntity;
class PhysObj;
class PhysCollisionObject;
class DLLNETWORK TraceFilterBase
{
public:
	virtual TraceFilterBase *Copy() const=0;
	virtual PhysClosestRayResultCallback CreateClosestRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const Vector3 &a,const Vector3 &b) const=0;
	virtual PhysAllHitsRayResultCallback CreateAllHitsRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const Vector3 &a,const Vector3 &b) const=0;
	virtual PhysClosestConvexResultCallback CreateClosestConvexCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const Vector3 &a,const Vector3 &b) const=0;
	virtual PhysContactResultCallback CreateContactCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask) const=0;
};
template<class T>
	class TraceFilter
		: public TraceFilterBase
{
private:
	T m_data;
public:
	TraceFilter(T t);
	virtual TraceFilterBase *Copy() const override;
	virtual PhysClosestRayResultCallback CreateClosestRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const Vector3 &a,const Vector3 &b) const override;
	virtual PhysAllHitsRayResultCallback CreateAllHitsRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const Vector3 &a,const Vector3 &b) const override;
	virtual PhysClosestConvexResultCallback CreateClosestConvexCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const Vector3 &a,const Vector3 &b) const override;
	virtual PhysContactResultCallback CreateContactCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask) const override;
};
#endif
namespace pragma::physics {class IConvexShape; class IRayCastFilterCallback;};
class DLLNETWORK TraceData
{
public:
	TraceData();
	TraceData(const TraceData &other);
#ifdef ENABLE_DEPRECATED_PHYSICS
	const TraceFilterBase *GetFilter() const;
#endif
	RayCastFlags GetFlags() const;
	const pragma::physics::Transform &GetSource() const;
	const pragma::physics::Transform &GetTarget() const;
	Vector3 GetSourceOrigin() const;
	Quat GetSourceRotation() const;
	Vector3 GetTargetOrigin() const;
	Quat GetTargetRotation() const;
	Vector3 GetDirection() const;
	float GetDistance() const;
	CollisionMask GetCollisionFilterMask() const;
	CollisionMask GetCollisionFilterGroup() const;
	void SetShape(const pragma::physics::IConvexShape &shape);
	const pragma::physics::IConvexShape *GetShape() const;
	void SetSource(const Vector3 &origin);
	void SetSourceRotation(const Quat &rot);
	void SetSource(const pragma::physics::Transform &t);
	void SetTarget(const Vector3 &target);
	void SetTargetRotation(const Quat &rot);
	void SetTarget(const pragma::physics::Transform &t);
	void SetRotation(const Quat &rot);
	void SetFlags(RayCastFlags flags);
	void SetCollisionFilterMask(CollisionMask mask);
	void SetCollisionFilterGroup(CollisionMask group);
	void SetFilter(const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &filter);
	const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &GetFilter() const;
#ifdef ENABLE_DEPRECATED_PHYSICS
	template<class T>
		void SetFilter(const T &filter);
#endif
	bool HasFlag(RayCastFlags flag) const;
protected:
	bool HasTarget() const;
private:
	pragma::physics::Transform m_tStart;
	pragma::physics::Transform m_tEnd;

	RayCastFlags m_flags;
	bool m_bHasTarget = false;
	CollisionMask m_filterMask;
	CollisionMask m_filterGroup;
	std::shared_ptr<pragma::physics::IRayCastFilterCallback> m_filter = nullptr;
	util::TWeakSharedHandle<pragma::physics::IConvexShape> m_shape = {};
#ifdef ENABLE_DEPRECATED_PHYSICS
	std::shared_ptr<TraceFilterBase> m_filter = nullptr;
#endif
};
#ifdef ENABLE_DEPRECATED_PHYSICS
template<class T>
	void TraceData::SetFilter(const T &filter)
{
	m_filter = std::shared_ptr<TraceFilter<T>>(new TraceFilter<T>(filter)); // c++14 std::make_unique<TraceFilter<T>>(filter);
}
#endif
class ModelMesh;
class ModelSubMesh;
class Material;
class PhysObj;
struct DLLNETWORK TraceResult
{
	TraceResult()
	{}
	TraceResult(const TraceData &data);
	RayCastHitType hitType = RayCastHitType::None;
	EntityHandle entity = {};
	util::TWeakSharedHandle<pragma::physics::ICollisionObject> collisionObj = {};
	std::weak_ptr<pragma::physics::IShape> shape = {};
	PhysObjHandle physObj = {};
	float fraction = 0.f;

	float distance = 0.f;
	Vector3 normal = {};
	Vector3 position = {};
	Vector3 startPosition = Vector3{};

	void GetMeshes(ModelMesh **mesh,ModelSubMesh **subMesh);
	Material *GetMaterial();
	bool GetMaterial(std::string &mat);
private:
	struct MeshInfo
	{
		std::vector<std::shared_ptr<ModelMesh>> meshes;
		ModelMesh *mesh = nullptr;
		ModelSubMesh *subMesh = nullptr;
	};
	std::shared_ptr<MeshInfo> m_meshInfo = nullptr;
	void InitializeMeshes();
};

#ifdef ENABLE_DEPRECATED_PHYSICS
template<class T>
	TraceFilter<T>::TraceFilter(T t)
		: m_data(t)
{}
template<class T>
	TraceFilterBase *TraceFilter<T>::Copy() const
{
	return new TraceFilter<T>(m_data);
}
template<class T>
	PhysClosestRayResultCallback TraceFilter<T>::CreateClosestRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const Vector3 &a,const Vector3 &b) const
{
	return PhysClosestRayResultCallback(a,b,flags,group,mask,m_data);
}
template<class T>
	PhysAllHitsRayResultCallback TraceFilter<T>::CreateAllHitsRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const Vector3 &a,const Vector3 &b) const
	{
		return PhysAllHitsRayResultCallback(a,b,flags,group,mask,m_data);
	}
template<class T>
	PhysClosestConvexResultCallback TraceFilter<T>::CreateClosestConvexCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const Vector3 &a,const Vector3 &b) const
	{
		return PhysClosestConvexResultCallback(a,b,flags,group,mask,m_data);
	}
template<class T>
	PhysContactResultCallback TraceFilter<T>::CreateContactCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask) const
	{
		return PhysContactResultCallback(flags,group,mask,m_data);
	}
#endif

#endif