#ifndef __RAYTRACES_H__
#define __RAYTRACES_H__
struct TraceResult;
#define ENGINE_RAYTRACES \
	public: \
		bool TraceRay(Vector3 &start,Vector3 &dir,unsigned int flags,std::deque<std::shared_ptr<TraceResult>> *res,std::vector<Entity*> *filter=NULL); \
		bool TraceAABB(Vector3 &start,Vector3 &dir,Vector3 &mins,Vector3 &maxs,unsigned int flags,std::deque<std::shared_ptr<TraceResult>> *res,std::vector<Entity*> *filter=NULL); \
		bool TraceOBB(); \
		bool TraceSphere();
#include <mathutil/glmutil.h>
#include <pragma/physics/physapi.h>
#include "pragma/physics/phystransform.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/raycallback/physraycallback.hpp"
#include "pragma/physics/raycallback/physraycallback_contact.hpp"
#include <memory>

enum class DLLNETWORK FTRACE
{
	ALL_HITS = 1,
	FILTER_INVERT = 2,
	IGNORE_DYNAMIC = 256,
	IGNORE_STATIC = 512
};
REGISTER_BASIC_BITWISE_OPERATORS(FTRACE);

class PhysEnv;
class BaseEntity;
class PhysObj;
class PhysCollisionObject;
class DLLNETWORK TraceFilterBase
{
public:
	virtual TraceFilterBase *Copy() const=0;
	virtual PhysClosestRayResultCallback CreateClosestRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const btVector3 &a,const btVector3 &b) const=0;
	virtual PhysAllHitsRayResultCallback CreateAllHitsRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const btVector3 &a,const btVector3 &b) const=0;
	virtual PhysClosestConvexResultCallback CreateClosestConvexCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const btVector3 &a,const btVector3 &b) const=0;
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
	virtual PhysClosestRayResultCallback CreateClosestRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const btVector3 &a,const btVector3 &b) const override;
	virtual PhysAllHitsRayResultCallback CreateAllHitsRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const btVector3 &a,const btVector3 &b) const override;
	virtual PhysClosestConvexResultCallback CreateClosestConvexCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const btVector3 &a,const btVector3 &b) const override;
	virtual PhysContactResultCallback CreateContactCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask) const override;
};

class DLLNETWORK TraceData
{
public:
	friend PhysEnv;
private:
	PhysTransform m_tStart;
	PhysTransform m_tEnd;
	void *m_target;
	PhysObjHandle m_phys;
	EntityHandle m_entity;
	UChar m_targetType;
	FTRACE m_flags;
	bool m_bHasTarget;
	CollisionMask m_filterMask;
	CollisionMask m_filterGroup;
	std::shared_ptr<TraceFilterBase> m_filter;
protected:
	const btConvexShape *GetShape() const;
	const PhysObj *GetPhysObj() const;
	const PhysCollisionObject *GetCollisionObject() const;
	Bool GetCollisionObjects(std::vector<PhysCollisionObject*> &objs) const;
	Bool GetShapes(std::vector<btConvexShape*> &shapes) const;
	const BaseEntity *GetEntity() const;
	bool HasTarget() const;
public:
	TraceData();
	TraceData(const TraceData &other);
	const TraceFilterBase *GetFilter() const;
	FTRACE GetFlags() const;
	const PhysTransform &GetSource() const;
	const PhysTransform &GetTarget() const;
	Vector3 GetSourceOrigin() const;
	Quat GetSourceRotation() const;
	Vector3 GetTargetOrigin() const;
	Quat GetTargetRotation() const;
	Vector3 GetDirection() const;
	float GetDistance() const;
	CollisionMask GetCollisionFilterMask() const;
	CollisionMask GetCollisionFilterGroup() const;
	void SetSource(btConvexShape *shape);
	void SetSource(PhysObj *obj);
	void SetSource(PhysCollisionObject *obj);
	void SetSource(const Vector3 &origin);
	void SetSource(BaseEntity *ent);
	void SetSourceRotation(const Quat &rot);
	void SetSource(const PhysTransform &t);
	void SetTarget(const Vector3 &target);
	void SetTargetRotation(const Quat &rot);
	void SetTarget(const PhysTransform &t);
	void SetRotation(const Quat &rot);
	void SetFlags(FTRACE flags);
	void SetCollisionFilterMask(CollisionMask mask);
	void SetCollisionFilterGroup(CollisionMask group);
	template<class T>
		void SetFilter(const T &filter);
	bool HasFlag(FTRACE flag) const;
};

template<class T>
	void TraceData::SetFilter(const T &filter)
{
	m_filter = std::shared_ptr<TraceFilter<T>>(new TraceFilter<T>(filter)); // c++14 std::make_unique<TraceFilter<T>>(filter);
}

class PhysObj;
struct DLLNETWORK TraceResult
{
	TraceResult()
#ifdef PHYS_ENGINE_PHYSX
		: flags(0),actor(NULL),shape(NULL)
#endif
	{}
	bool hit = false;
	EntityHandle entity = {};
	PhysCollisionObjectHandle collisionObj = {};
	PhysObjHandle physObj = {};
	float fraction = 0.f;
#ifdef PHYS_ENGINE_PHYSX
	physx::PxRigidActor *actor;
	physx::PxHitFlags flags;
	physx::PxShape *shape;
#endif
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
	PhysClosestRayResultCallback TraceFilter<T>::CreateClosestRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const btVector3 &a,const btVector3 &b) const
{
	return PhysClosestRayResultCallback(a,b,flags,group,mask,m_data);
}
template<class T>
	PhysAllHitsRayResultCallback TraceFilter<T>::CreateAllHitsRayCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const btVector3 &a,const btVector3 &b) const
	{
		return PhysAllHitsRayResultCallback(a,b,flags,group,mask,m_data);
	}
template<class T>
	PhysClosestConvexResultCallback TraceFilter<T>::CreateClosestConvexCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask,const btVector3 &a,const btVector3 &b) const
	{
		return PhysClosestConvexResultCallback(a,b,flags,group,mask,m_data);
	}
template<class T>
	PhysContactResultCallback TraceFilter<T>::CreateContactCallbackFilter(FTRACE flags,CollisionMask group,CollisionMask mask) const
	{
		return PhysContactResultCallback(flags,group,mask,m_data);
	}

#endif