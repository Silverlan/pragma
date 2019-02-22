#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/physics/raytraces.h"
#include "pragma/physics/collisiontypes.h"
#include <mathutil/color.h>
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/physshape.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/model/modelmesh.h"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/model/model.h"
#include <pragma/math/intersection.h>

void TraceResult::InitializeMeshes()
{
	if(m_meshInfo != nullptr)
		return;
	m_meshInfo = std::make_shared<MeshInfo>();
	if(entity.IsValid() == false)
		return;
	auto mdlComponent = entity->GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return;
	hMdl->GetBodyGroupMeshes(mdlComponent->GetBodyGroups(),0u,m_meshInfo->meshes);
	auto closestDist = std::numeric_limits<float>::max();

	auto pTrComponent = entity->GetTransformComponent();
	auto origin = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	auto rot = pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity();

	auto startPosLocal = startPosition;
	uvec::world_to_local(origin,rot,startPosLocal);

	auto maxDist = distance +50.f;
	auto dir = position -startPosition;
	uvec::normalize(&dir);
	uvec::world_to_local(origin,rot,dir);
	dir *= maxDist;

	auto tClosest = std::numeric_limits<float>::max();
	for(auto &mesh : m_meshInfo->meshes)
	{
		Vector3 min,max;
		mesh->GetBounds(min,max);
		auto dist = umath::min(maxDist,tClosest);
		auto t = 0.f;
		if(Intersection::LineAABB(startPosLocal,dir,min,max,&t) != Intersection::Result::Intersect || umath::abs(t) > dist)
			continue;
		auto &subMeshes = mesh->GetSubMeshes();
		for(auto &subMesh : subMeshes)
		{
			subMesh->GetBounds(min,max);
			if(Intersection::LineAABB(startPosLocal,dir,min,max,&t) != Intersection::Result::Intersect || umath::abs(t) > dist)
				continue;
			if(Intersection::LineMesh(startPosLocal,dir,*subMesh,nullptr,nullptr,&t) != Intersection::Result::Intersect || umath::abs(t) > dist)
				continue;
			tClosest = umath::abs(t);
			m_meshInfo->mesh = mesh.get();
			m_meshInfo->subMesh = subMesh.get();
		}
	}
}

void TraceResult::GetMeshes(ModelMesh **outMesh,ModelSubMesh **outSubMesh)
{
	InitializeMeshes();
	*outMesh = m_meshInfo->mesh;
	*outSubMesh = m_meshInfo->subMesh;
}
Material *TraceResult::GetMaterial()
{
	if(entity.IsValid() == false)
		return nullptr;
	auto mdlComponent = entity->GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return nullptr;
	ModelMesh *mesh = nullptr;
	ModelSubMesh *subMesh = nullptr;
	GetMeshes(&mesh,&subMesh);
	if(mesh == nullptr || subMesh == nullptr)
		return nullptr;
	auto texId = subMesh->GetTexture();
	auto &materials = hMdl->GetMaterials();
	if(texId >= materials.size())
		return nullptr;
	return materials.at(texId).get();
}
bool TraceResult::GetMaterial(std::string &mat)
{
	if(entity.IsValid() == false)
		return false;
	auto mdlComponent = entity->GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return false;
	ModelMesh *mesh = nullptr;
	ModelSubMesh *subMesh = nullptr;
	GetMeshes(&mesh,&subMesh);
	if(mesh == nullptr || subMesh == nullptr)
		return false;
	auto texId = subMesh->GetTexture();
	auto &textures = hMdl->GetTextures();
	if(texId >= textures.size())
		return false;
	mat = textures.at(texId);
	return true;
}

/////////////////////////////////////////

TraceData::TraceData()
	: m_targetType(0),m_flags(FTRACE(0)),m_filterMask(CollisionMask::All),m_filterGroup(CollisionMask::Default),m_filter(nullptr),m_bHasTarget(false)
{
	m_tStart.SetIdentity();
	m_tEnd.SetIdentity();
}
TraceData::TraceData(const TraceData &other)
	: m_tStart(other.m_tStart),m_tEnd(other.m_tEnd),
	m_target(other.m_target),m_phys(other.m_phys),
	m_entity(other.m_entity),m_targetType(other.m_targetType),
	m_flags(other.m_flags),m_bHasTarget(other.m_bHasTarget),
	m_filterMask(other.m_filterMask),m_filterGroup(other.m_filterGroup),
	m_filter(std::shared_ptr<TraceFilterBase>(other.m_filter->Copy()))
{}
const btConvexShape *TraceData::GetShape() const
{
	if(m_targetType != 1)
		return nullptr;
	return static_cast<btConvexShape*>(m_target);
}
const PhysObj *TraceData::GetPhysObj() const
{
	if(m_targetType != 2 || !m_phys.IsValid())
		return nullptr;
	return m_phys.get();
}
const PhysCollisionObject *TraceData::GetCollisionObject() const
{
	if(m_targetType != 3)
		return nullptr;
	return static_cast<PhysCollisionObject*>(m_target);
}
const BaseEntity *TraceData::GetEntity() const
{
	if(m_targetType != 4 || !m_entity.IsValid())
		return nullptr;
	return m_entity.get();
}
Vector3 TraceData::GetSourceOrigin() const
{
	auto &t = GetSource();
	return t.GetOrigin();
}
Quat TraceData::GetSourceRotation() const
{
	auto &t = GetSource();
	return t.GetRotation();
}
Vector3 TraceData::GetTargetOrigin() const {return m_tEnd.GetOrigin();}
Quat TraceData::GetTargetRotation() const {return m_tEnd.GetRotation();}
bool TraceData::HasTarget() const {return m_bHasTarget;}
Bool TraceData::GetCollisionObjects(std::vector<PhysCollisionObject*> &objs) const
{
	if(m_targetType <= 1)
		return false;
	auto *obj = GetCollisionObject();
	if(obj != nullptr)
	{
		objs.push_back(const_cast<PhysCollisionObject*>(obj));
		return true;
	}
	auto *phys = GetPhysObj();
	if(phys == nullptr)
	{
		auto *ent = GetEntity();
		if(ent != nullptr)
		{
			auto pPhysComponentEnt = ent->GetPhysicsComponent();
			phys = pPhysComponentEnt.valid() ? pPhysComponentEnt->GetPhysicsObject() : nullptr;
		}
	}
	if(phys == nullptr)
		return false;
	auto &physObjs = const_cast<PhysObj*>(phys)->GetCollisionObjects();
	objs.reserve(physObjs.size());
	for(auto it=physObjs.begin();it!=physObjs.end();++it)
	{
		auto &hObj = *it;
		if(hObj.IsValid())
			objs.push_back(hObj.get());
	}
	return true;
}
Bool TraceData::GetShapes(std::vector<btConvexShape*> &shapes) const
{
	if(m_targetType == 0)
		return false;
	auto *shape = GetShape();
	if(shape != nullptr)
	{
		shapes.push_back(const_cast<btConvexShape*>(shape));
		return true;
	}
	auto *obj = GetCollisionObject();
	if(obj != nullptr)
	{
		auto shape = obj->GetCollisionShape();
		if(shape == nullptr)
			return false;
		auto *btShape = shape->GetShape();
		if(!btShape->isConvex())
			return false;
		shapes.push_back(static_cast<btConvexShape*>(btShape));
		return true;
	}
	auto *phys = GetPhysObj();
	if(phys == nullptr)
	{
		auto *ent = GetEntity();
		if(ent == nullptr)
			return false;
		auto pPhysComponent = ent->GetPhysicsComponent();
		phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	}
	if(phys == nullptr)
		return false;
	auto &objs = const_cast<PhysObj*>(phys)->GetCollisionObjects();
	for(auto it=objs.begin();it!=objs.end();it++)
	{
		auto &hO = *it;
		if(hO.IsValid())
		{
			auto shape = hO->GetCollisionShape();
			if(shape != nullptr)
			{
				auto *btShape = shape->GetShape();
				if(btShape->isConvex())
					shapes.push_back(static_cast<btConvexShape*>(btShape));
			}
		}
	}
	return true;
}
const PhysTransform &TraceData::GetSource() const
{
	if(m_targetType > 1)
	{
		auto *phys = const_cast<PhysObj*>(GetPhysObj());
		if(phys != nullptr)
		{
			const_cast<TraceData*>(this)->m_tStart.SetOrigin(phys->GetPosition());
			const_cast<TraceData*>(this)->m_tStart.SetRotation(phys->GetOrientation());
		}
		else
		{
			auto *col = const_cast<PhysCollisionObject*>(GetCollisionObject());
			if(col != nullptr)
			{
				const_cast<TraceData*>(this)->m_tStart.SetOrigin(col->GetPos());
				const_cast<TraceData*>(this)->m_tStart.SetRotation(col->GetRotation());
			}
			else
			{
				auto *ent = GetEntity();
				if(ent != nullptr)
				{
					auto pTrComponent = ent->GetTransformComponent();
					const_cast<TraceData*>(this)->m_tStart.SetOrigin(pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{});
					const_cast<TraceData*>(this)->m_tStart.SetRotation(pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity());
				}
			}
		}
	}
	return m_tStart;
}
const PhysTransform &TraceData::GetTarget() const {return m_tEnd;}
CollisionMask TraceData::GetCollisionFilterMask() const {return m_filterMask;}
CollisionMask TraceData::GetCollisionFilterGroup() const {return m_filterGroup;}
void TraceData::SetSource(btConvexShape *shape)
{
	m_targetType = 1;
	m_target = shape;
}
void TraceData::SetSource(PhysObj *obj)
{
	m_targetType = 2;
	m_phys = obj->GetHandle();
}
void TraceData::SetSource(PhysCollisionObject *obj)
{
	m_targetType = 3;
	m_target = obj;
}
void TraceData::SetSource(BaseEntity *obj)
{
	m_targetType = 4;
	m_entity = obj->GetHandle();
}
void TraceData::SetSource(const Vector3 &origin) {m_targetType = 0;m_tStart.SetOrigin(origin);}
void TraceData::SetSourceRotation(const Quat &rot) {m_tStart.SetRotation(rot);}
void TraceData::SetSource(const PhysTransform &t) {m_targetType = 0;m_tStart = t;}
void TraceData::SetTarget(const Vector3 &target) {m_tEnd.SetOrigin(target);m_bHasTarget = true;}
void TraceData::SetTargetRotation(const Quat &rot) {m_tEnd.SetRotation(rot);m_bHasTarget = true;}
void TraceData::SetTarget(const PhysTransform &t) {m_tEnd = t;m_bHasTarget = true;}
void TraceData::SetRotation(const Quat &rot)
{
	SetSourceRotation(rot);
	SetTargetRotation(rot);
}
void TraceData::SetFlags(FTRACE flags) {m_flags = flags;}
FTRACE TraceData::GetFlags() const {return m_flags;}
const TraceFilterBase *TraceData::GetFilter() const {return m_filter.get();}
void TraceData::SetCollisionFilterMask(CollisionMask mask) {m_filterMask = mask;}
void TraceData::SetCollisionFilterGroup(CollisionMask group) {m_filterGroup = group;}
Vector3 TraceData::GetDirection() const
{
	auto src = GetSourceOrigin();
	auto tgt = GetTargetOrigin();
	auto dir = tgt -src;
	uvec::normalize(&dir);
	return dir;
}
float TraceData::GetDistance() const
{
	auto src = GetSourceOrigin();
	auto tgt = GetTargetOrigin();
	return uvec::distance(src,tgt);
}
bool TraceData::HasFlag(FTRACE flag) const {return ((UInt32(m_flags) &UInt32(flag)) != 0) ? true : false;}

#ifdef PHYS_ENGINE_BULLET
Bool Game::Overlap(const TraceData &data,TraceResult *result) {return m_physEnvironment->Overlap(data,result);}
TraceResult Game::Overlap(const TraceData &data)
{
	TraceResult result;
	Overlap(data,&result);
	return result;
}
Bool Game::RayCast(const TraceData &data,std::vector<TraceResult> *results) {return m_physEnvironment->RayCast(data,results);}
TraceResult Game::RayCast(const TraceData &data)
{
	std::vector<TraceResult> results;
	RayCast(data,&results);
	if(results.empty())
	{
		auto r = TraceResult{};
		r.position = data.GetTargetOrigin();
		return r;
	}
	return results.front();
}
Bool Game::Sweep(const TraceData &data,TraceResult *result) {return m_physEnvironment->Sweep(data,result);}
TraceResult Game::Sweep(const TraceData &data)
{
	TraceResult result;
	Sweep(data,&result);
	return result;
}
#elif PHYS_ENGINE_PHYSX
static struct PxFilteredQueryFilterCallback
	: physx::PxQueryFilterCallback
{
protected:
	std::vector<physx::PxRigidActor*> m_filter;
	bool m_bInvert;
	void AddFilter(BaseEntity *filter)
	{
		PhysObj *phys = filter->GetPhysicsObject();
		if(phys == NULL)
			return;
		std::vector<physx::PxRigidActor*> *actors = phys->GetActors();
		for(unsigned int i=0;i<actors->size();i++)
			m_filter.push_back((*actors)[i]);
	}
	void AddFilter(physx::PxRigidActor *actor)
	{
		m_filter.push_back(actor);
	}
public:
	PxFilteredQueryFilterCallback(BaseEntity *filter,bool bInvert=false)
		: m_bInvert(bInvert)
	{
		AddFilter(filter);
	}
	PxFilteredQueryFilterCallback(physx::PxRigidActor *actor,bool bInvert=false)
		: m_bInvert(bInvert)
	{
		AddFilter(actor);
	}
	PxFilteredQueryFilterCallback(std::vector<physx::PxRigidActor*> &actors,bool bInvert=false)
		: m_bInvert(bInvert)
	{
		for(unsigned int i=0;i<actors.size();i++)
			AddFilter(actors[i]);
	}
	PxFilteredQueryFilterCallback(std::vector<BaseEntity*> &filter,bool bInvert=false)
		: m_bInvert(bInvert)
	{
		for(unsigned int i=0;i<filter.size();i++)
			AddFilter(filter[i]);
	}
	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData,const physx::PxShape* shape,const physx::PxRigidActor *actor,physx::PxHitFlags &queryFlags)
	{
		for(unsigned int i=0;i<m_filter.size();i++)
			if(m_filter[i]->userData == actor->userData)
				return physx::PxQueryHitType::Enum::eNONE;
		return physx::PxQueryHitType::Enum::eBLOCK;
	}
	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData,const physx::PxQueryHit &hit)
	{
		for(unsigned int i=0;i<m_filter.size();i++)
		{
			if((m_bInvert == false && m_filter[i]->userData == hit.actor->userData) || (m_bInvert == true && m_filter[i]->userData != hit.actor->userData))
				return physx::PxQueryHitType::Enum::eNONE;
		}
		return physx::PxQueryHitType::Enum::eBLOCK;
	}
};

static struct PxFilteredQueryFilterFunctionCallback
	: physx::PxQueryFilterCallback
{
protected:
	physx::PxQueryHitType::Enum(*m_preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&);
	physx::PxQueryHitType::Enum(*m_postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&);
public:
	PxFilteredQueryFilterFunctionCallback(physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&))
		: m_preFilter(preFilter),m_postFilter(NULL)
	{}
	PxFilteredQueryFilterFunctionCallback(physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&))
		: m_preFilter(NULL),m_postFilter(postFilter)
	{}
	PxFilteredQueryFilterFunctionCallback(physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&))
		: m_preFilter(preFilter),m_postFilter(postFilter)
	{}
	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData,const physx::PxShape* shape,const physx::PxRigidActor *actor,physx::PxHitFlags &queryFlags)
	{
		return preFilter(filterData,shape,actor,queryFlags);
	}
	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData,const physx::PxQueryHit &hit)
	{
		return postFilter(filterData,hit);
	}
};

static struct PxFilteredQueryFilterActorCallback
	: physx::PxQueryFilterCallback
{
protected:
	physx::PxQueryHitType::Enum(*m_filter)(const physx::PxRigidActor*);
public:
	PxFilteredQueryFilterActorCallback(physx::PxQueryHitType::Enum(*filter)(const physx::PxRigidActor*))
		: m_filter(filter)
	{}
	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData,const physx::PxShape* shape,const physx::PxRigidActor *actor,physx::PxHitFlags &queryFlags)
	{
		return m_filter(actor);
	}
	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData,const physx::PxQueryHit &hit)
	{
		return m_filter(hit.actor);
	}
};

static struct PxFilteredQueryFilterPhysObjCallback
	: physx::PxQueryFilterCallback
{
protected:
	physx::PxQueryHitType::Enum(*m_filter)(PhysObj*);
public:
	PxFilteredQueryFilterPhysObjCallback(physx::PxQueryHitType::Enum(*filter)(PhysObj*))
		: m_filter(filter)
	{}
	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData,const physx::PxShape* shape,const physx::PxRigidActor *actor,physx::PxHitFlags &queryFlags)
	{
		void *data = actor->userData;
		if(data == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		PhysObj *phys = static_cast<PhysObj*>(data);
		return m_filter(phys);
	}
	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData,const physx::PxQueryHit &hit)
	{
		void *data = hit.actor->userData;
		if(data == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		PhysObj *phys = static_cast<PhysObj*>(data);
		return m_filter(phys);
	}
};

static struct PxFilteredQueryFilterEntityCallback
	: physx::PxQueryFilterCallback
{
protected:
	physx::PxQueryHitType::Enum(*m_filter)(BaseEntity*);
public:
	PxFilteredQueryFilterEntityCallback(physx::PxQueryHitType::Enum(*filter)(BaseEntity*))
		: m_filter(filter)
	{}
	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData,const physx::PxShape* shape,const physx::PxRigidActor *actor,physx::PxHitFlags &queryFlags)
	{
		void *data = actor->userData;
		if(data == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		PhysObj *phys = static_cast<PhysObj*>(data);
		BaseEntity *owner = phys->GetOwner();
		if(owner == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		return m_filter(owner);
	}
	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData,const physx::PxQueryHit &hit)
	{
		void *data = hit.actor->userData;
		if(data == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		PhysObj *phys = static_cast<PhysObj*>(data);
		BaseEntity *owner = phys->GetOwner();
		if(owner == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		return m_filter(owner);
	}
};

static void InitializeFlags(unsigned int flags,physx::PxQueryFlags &queryFlags)
{
	if((flags &FTRACE_ANY_HIT) == FTRACE_ANY_HIT)
		queryFlags |= physx::PxQueryFlag::eANY_HIT;
	if((flags &FTRACE_NO_BLOCK) == FTRACE_NO_BLOCK)
		queryFlags |= physx::PxQueryFlag::eNO_BLOCK;
	if((flags &FTRACE_PREFILTER) == FTRACE_PREFILTER)
		queryFlags |= physx::PxQueryFlag::ePREFILTER;
	if((flags &FTRACE_POSTFILTER) == FTRACE_POSTFILTER)
		queryFlags |= physx::PxQueryFlag::ePOSTFILTER;
	if((flags &FTRACE_IGNORE_DYNAMIC) == 0)
		queryFlags |= physx::PxQueryFlag::eDYNAMIC;
	if((flags &FTRACE_IGNORE_STATIC) == 0)
		queryFlags |= physx::PxQueryFlag::eSTATIC;
}

static void InitializeFlags(unsigned int flags,physx::PxHitFlags &hitFlags,physx::PxQueryFlags &queryFlags)
{
	InitializeFlags(flags,queryFlags);
	hitFlags |= (physx::PxHitFlag::eDISTANCE | physx::PxHitFlag::eNORMAL | physx::PxHitFlag::ePOSITION);
	if((flags &FTRACE_ASSUME_NO_INITIAL_OVERLAP) == FTRACE_ASSUME_NO_INITIAL_OVERLAP)
		hitFlags |= physx::PxHitFlag::eASSUME_NO_INITIAL_OVERLAP;
	if((flags &FTRACE_MESH_ANY) == FTRACE_MESH_ANY)
		hitFlags |= physx::PxHitFlag::eMESH_ANY;
	if((flags &FTRACE_MESH_BOTH_SIDES) == FTRACE_MESH_BOTH_SIDES)
		hitFlags |= physx::PxHitFlag::eMESH_BOTH_SIDES;
	if((flags &FTRACE_MESH_MULTIPLE) == FTRACE_MESH_MULTIPLE)
		hitFlags |= physx::PxHitFlag::eMESH_MULTIPLE;
	if((flags &FTRACE_PRECISE_SWEEP) == FTRACE_PRECISE_SWEEP)
		hitFlags |= physx::PxHitFlag::ePRECISE_SWEEP;
}

template<class TBuffer,class THit>
	static void FillTraceResult(TraceResult *res,TBuffer &hit)
{
	if(hit.hasBlock == true)
	{
		res->hit = true;
		physx::PxRigidActor *actor = hit.block.actor;
		void *data = actor->userData;
		if(data != NULL)
		{
			PhysObj *phys = static_cast<PhysObj*>(data);
			BaseEntity *owner = phys->GetOwner();
			res->physObj = phys;
			if(owner != NULL)
				res->entity = owner;
		}
		res->distance = hit.block.distance;
		res->flags = hit.block.flags;
		res->normal = Vector3(hit.block.normal.x,hit.block.normal.y,hit.block.normal.z);
		res->position = Vector3(hit.block.position.x,hit.block.position.y,hit.block.position.z);
		res->shape = hit.block.shape;
		res->hadInitialOverlap = hit.block.hadInitialOverlap();
		res->faceIndex = hit.block.faceIndex;
	}
	else
		res->hit = false;
	for(unsigned int i=0;i<hit.nbTouches;i++)
	{
		THit &touch = hit.touches[i];
		res->touches.push_back(TraceResult());
		TraceResult &resTouch = res->touches.back();
		resTouch.distance = touch.distance;
		resTouch.flags = touch.flags;
		resTouch.normal = Vector3(touch.normal.x,touch.normal.y,touch.normal.z);
		resTouch.position = Vector3(touch.position.x,touch.position.y,touch.position.z);
		resTouch.shape = touch.shape;
		resTouch.hadInitialOverlap = touch.hadInitialOverlap();
		resTouch.faceIndex = touch.faceIndex;
	}
}

static void FillOverlapResult(TraceResult *res,physx::PxOverlapBuffer &hit)
{
	if(hit.hasBlock == true)
	{
		res->hit = true;
		physx::PxRigidActor *actor = hit.block.actor;
		void *data = actor->userData;
		if(data != NULL)
		{
			PhysObj *phys = static_cast<PhysObj*>(data);
			BaseEntity *owner = phys->GetOwner();
			res->physObj = phys;
			if(owner != NULL)
				res->entity = owner;
		}
		res->shape = hit.block.shape;
		res->faceIndex = hit.block.faceIndex;
	}
	else
		res->hit = false;
	for(unsigned int i=0;i<hit.nbTouches;i++)
	{
		physx::PxOverlapHit &touch = hit.touches[i];
		res->touches.push_back(TraceResult());
		TraceResult &resTouch = res->touches.back();
		resTouch.shape = touch.shape;
		resTouch.faceIndex = touch.faceIndex;
	}
}

static void FillTraceResult(TraceResult *res,physx::PxRaycastBuffer &hit)
{
	FillTraceResult<physx::PxRaycastBuffer,physx::PxRaycastHit>(res,hit);
	if(hit.hasBlock == true)
	{
		res->u = hit.block.u;
		res->v = hit.block.v;
	}
	else
		res->hit = false;
	for(unsigned int i=0;i<hit.nbTouches;i++)
	{
		physx::PxRaycastHit &touch = hit.touches[i];
		TraceResult &resTouch = res->touches[i];
		resTouch.u = touch.u;
		resTouch.v = touch.v;
	}
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,TraceResult *res,unsigned int flags)
{
	physx::PxScene *scene = GetPhysXScene();
	physx::PxRaycastHit *touches = NULL;
	physx::PxQueryFilterData fd;
	physx::PxHitFlags hitFlags;
	InitializeFlags(flags,hitFlags,fd.flags);
	bool bAnyHit = ((fd.flags &physx::PxQueryFlag::eANY_HIT) == physx::PxQueryFlag::eANY_HIT) ? true : false;
	unsigned int numTouches = 0;
	if(bAnyHit == false && (flags &FTRACE_TOUCHING_HITS) == FTRACE_TOUCHING_HITS)
	{
		numTouches = 64;
		touches = new physx::PxRaycastHit[numTouches];
	}
	physx::PxRaycastBuffer hit(touches,numTouches);

	bool b = scene->raycast(
		physx::PxVec3(origin.x,origin.y,origin.z),
		physx::PxVec3(dir.x,dir.y,dir.z),
		distance,
		hit,
		hitFlags,
		fd
	);
	if(res != NULL)
		FillTraceResult(res,hit);
	if(res == NULL || b == false || bAnyHit == true)
	{
		if(touches != NULL)
			delete[] touches;
		return b;
	}
	if(touches != NULL)
		delete[] touches;
	return true;
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryFilterCallback &filter,TraceResult *res,unsigned int flags)
{
	physx::PxScene *scene = GetPhysXScene();
	physx::PxRaycastHit *touches = NULL;
	physx::PxQueryFilterData fd;
	physx::PxHitFlags hitFlags;
	InitializeFlags(flags,hitFlags,fd.flags);
	bool bAnyHit = ((fd.flags &physx::PxQueryFlag::eANY_HIT) == physx::PxQueryFlag::eANY_HIT) ? true : false;
	unsigned int numTouches = 0;
	if(bAnyHit == false && (flags &FTRACE_TOUCHING_HITS) == FTRACE_TOUCHING_HITS)
	{
		numTouches = 64;
		touches = new physx::PxRaycastHit[numTouches];
	}
	physx::PxRaycastBuffer hit(touches,numTouches);
	bool b = scene->raycast(
		physx::PxVec3(origin.x,origin.y,origin.z),
		physx::PxVec3(dir.x,dir.y,dir.z),
		distance,
		hit,
		hitFlags,
		fd,
		&filter
	);
	if(res != NULL)
		FillTraceResult(res,hit);
	if(res == NULL || b == false || bAnyHit == true)
	{
		if(touches != NULL)
			delete[] touches;
		return b;
	}
	if(touches != NULL)
		delete[] touches;
	return true;
}

template<class T>
	bool Game::FilteredRayCast(Vector3 &origin,Vector3 &dir,float distance,T filter,TraceResult *res,unsigned int flags)
{
	return RayCast(origin,dir,distance,PxFilteredQueryFilterCallback(filter),res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,BaseEntity *filter,TraceResult *res,unsigned int flags)
{
	if(filter == NULL)
		return RayCast(origin,dir,distance,res,flags);
	return FilteredRayCast<BaseEntity*>(origin,dir,distance,filter,res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,EntityHandle &hFilter,TraceResult *res,unsigned int flags)
{
	return RayCast(origin,dir,distance,hFilter.get(),res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,std::vector<BaseEntity*> &filter,TraceResult *res,unsigned int flags)
{
	return FilteredRayCast<std::vector<BaseEntity*>&>(origin,dir,distance,filter,res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,physx::PxRigidActor *filter,TraceResult *res,unsigned int flags)
{
	return FilteredRayCast<physx::PxRigidActor*>(origin,dir,distance,filter,res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,std::vector<physx::PxRigidActor*> &filter,TraceResult *res,unsigned int flags)
{
	return FilteredRayCast<std::vector<physx::PxRigidActor*>&>(origin,dir,distance,filter,res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),TraceResult *res,unsigned int flags)
{
	return RayCast(origin,dir,distance,PxFilteredQueryFilterFunctionCallback(preFilter),res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags)
{
	return RayCast(origin,dir,distance,PxFilteredQueryFilterFunctionCallback(postFilter),res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags)
{
	return RayCast(origin,dir,distance,PxFilteredQueryFilterFunctionCallback(preFilter,postFilter),res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*filter)(BaseEntity*),TraceResult *res,unsigned int flags)
{
	return RayCast(origin,dir,distance,PxFilteredQueryFilterEntityCallback(filter),res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*filter)(const physx::PxRigidActor*),TraceResult *res,unsigned int flags)
{
	return RayCast(origin,dir,distance,PxFilteredQueryFilterActorCallback(filter),res,flags);
}

bool Game::RayCast(Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*filter)(PhysObj*),TraceResult *res,unsigned int flags)
{
	return RayCast(origin,dir,distance,PxFilteredQueryFilterPhysObjCallback(filter),res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,physx::PxQueryFilterCallback &filter,TraceResult *res,unsigned int flags)
{
	flags |= FTRACE_PREFILTER;
	physx::PxTransform t;
	t.p = physx::PxVec3(origin.x,origin.y,origin.z);
	t.q = physx::PxQuat(rot.x,rot.y,rot.z,rot.w);
	physx::PxScene *scene = GetPhysXScene();
	physx::PxSweepHit *touches = NULL;
	physx::PxQueryFilterData fd;
	physx::PxHitFlags hitFlags;
	InitializeFlags(flags,hitFlags,fd.flags);
	bool bAnyHit = ((fd.flags &physx::PxQueryFlag::eANY_HIT) == physx::PxQueryFlag::eANY_HIT) ? true : false;
	unsigned int numTouches = 0;
	if(bAnyHit == false && (flags &FTRACE_TOUCHING_HITS) == FTRACE_TOUCHING_HITS)
	{
		numTouches = 64;
		touches = new physx::PxSweepHit[numTouches];
	}
	physx::PxSweepBuffer hit(touches,numTouches);
	bool b = scene->sweep(
		geometry,
		t,
		physx::PxVec3(dir.x,dir.y,dir.z),
		distance,
		hit,
		hitFlags,
		fd,
		&filter
	);
	if(res != NULL)
		FillTraceResult<physx::PxSweepBuffer,physx::PxSweepHit>(res,hit);
	if(res == NULL || b == false || bAnyHit == true)
	{
		if(touches != NULL)
			delete[] touches;
		return b;
	}
	if(touches != NULL)
		delete[] touches;
	return true;
}

template<class T>
	bool Game::FilteredSweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,T filter,TraceResult *res,unsigned int flags)
{
	return Sweep(geometry,origin,rot,dir,distance,PxFilteredQueryFilterCallback(filter),res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,TraceResult *res,unsigned int flags)
{
	physx::PxTransform t;
	t.p = physx::PxVec3(origin.x,origin.y,origin.z);
	t.q = physx::PxQuat(rot.x,rot.y,rot.z,rot.w);
	physx::PxScene *scene = GetPhysXScene();
	physx::PxSweepHit *touches = NULL;
	physx::PxQueryFilterData fd;
	physx::PxHitFlags hitFlags;
	InitializeFlags(flags,hitFlags,fd.flags);
	bool bAnyHit = ((fd.flags &physx::PxQueryFlag::eANY_HIT) == physx::PxQueryFlag::eANY_HIT) ? true : false;
	unsigned int numTouches = 0;
	if(bAnyHit == false && (flags &FTRACE_TOUCHING_HITS) == FTRACE_TOUCHING_HITS)
	{
		numTouches = 64;
		touches = new physx::PxSweepHit[numTouches];
	}
	physx::PxSweepBuffer hit(touches,numTouches);
	bool b = scene->sweep(
		geometry,
		t,
		physx::PxVec3(dir.x,dir.y,dir.z),
		distance,
		hit,
		hitFlags,
		fd
	);
	if(res != NULL)
		FillTraceResult<physx::PxSweepBuffer,physx::PxSweepHit>(res,hit);
	if(res == NULL || b == false || bAnyHit == true)
	{
		if(touches != NULL)
			delete[] touches;
		return b;
	}
	if(touches != NULL)
		delete[] touches;
	return true;
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,BaseEntity *filter,TraceResult *res,unsigned int flags)
{
	if(filter == NULL)
		return Sweep(geometry,origin,rot,dir,distance,res,flags);
	return FilteredSweep<BaseEntity*>(geometry,origin,rot,dir,distance,filter,res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,EntityHandle &hFilter,TraceResult *res,unsigned int flags)
{
	return Sweep(geometry,origin,rot,dir,distance,hFilter.get(),res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,std::vector<BaseEntity*> &filter,TraceResult *res,unsigned int flags)
{
	return FilteredSweep<std::vector<BaseEntity*>&>(geometry,origin,rot,dir,distance,filter,res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,physx::PxRigidActor *filter,TraceResult *res,unsigned int flags)
{
	return FilteredSweep<physx::PxRigidActor*>(geometry,origin,rot,dir,distance,filter,res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,std::vector<physx::PxRigidActor*> &filter,TraceResult *res,unsigned int flags)
{
	return FilteredSweep<std::vector<physx::PxRigidActor*>&>(geometry,origin,rot,dir,distance,filter,res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),TraceResult *res,unsigned int flags)
{
	return Sweep(geometry,origin,rot,dir,distance,PxFilteredQueryFilterFunctionCallback(preFilter),res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags)
{
	return Sweep(geometry,origin,rot,dir,distance,PxFilteredQueryFilterFunctionCallback(postFilter),res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags)
{
	return Sweep(geometry,origin,rot,dir,distance,PxFilteredQueryFilterFunctionCallback(preFilter,postFilter),res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*filter)(BaseEntity*),TraceResult *res,unsigned int flags)
{
	return Sweep(geometry,origin,rot,dir,distance,PxFilteredQueryFilterEntityCallback(filter),res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*filter)(const physx::PxRigidActor*),TraceResult *res,unsigned int flags)
{
	return Sweep(geometry,origin,rot,dir,distance,PxFilteredQueryFilterActorCallback(filter),res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*filter)(PhysObj*),TraceResult *res,unsigned int flags)
{
	return Sweep(geometry,origin,rot,dir,distance,PxFilteredQueryFilterPhysObjCallback(filter),res,flags);
}

bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,BaseEntity *filter,TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,filter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,EntityHandle &hFilter,TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,hFilter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,std::vector<BaseEntity*> &filter,TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,filter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,physx::PxRigidActor *filter,TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,filter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,std::vector<physx::PxRigidActor*> &filter,TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,filter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryFilterCallback &filter,TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,filter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,preFilter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,postFilter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,preFilter,postFilter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*filter)(BaseEntity*),TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,filter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*filter)(const physx::PxRigidActor*),TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,filter,res,flags);}
bool Game::Sweep(const physx::PxGeometry &geometry,Vector3 &origin,Vector3 &dir,float distance,physx::PxQueryHitType::Enum(*filter)(PhysObj*),TraceResult *res,unsigned int flags) {return Sweep(geometry,origin,uquat::identity(),dir,distance,filter,res,flags);}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,TraceResult *res,unsigned int flags)
{
	physx::PxTransform t;
	t.p = physx::PxVec3(origin.x,origin.y,origin.z);
	t.q = physx::PxQuat(rot.x,rot.y,rot.z,rot.w);
	physx::PxQueryFilterData fd;
	InitializeFlags(flags,fd.flags);
	physx::PxScene *scene = GetPhysXScene();
	physx::PxOverlapHit *touches = NULL;
	bool bAnyHit = ((fd.flags &physx::PxQueryFlag::eANY_HIT) == physx::PxQueryFlag::eANY_HIT) ? true : false;
	unsigned int numTouches = 0;
	if(bAnyHit == false && (flags &FTRACE_TOUCHING_HITS) == FTRACE_TOUCHING_HITS)
	{
		numTouches = 64;
		touches = new physx::PxOverlapHit[numTouches];
	}
	physx::PxOverlapBuffer hit(touches,numTouches);
	bool b = scene->overlap(
		geometry,
		t,
		hit,
		fd
	);
	if(res != NULL)
		FillOverlapResult(res,hit);
	if(res == NULL || b == false || bAnyHit == true)
	{
		if(touches != NULL)
			delete[] touches;
		return b;
	}
	if(touches != NULL)
		delete[] touches;
	return true;
}

template<class T>
	bool Game::FilteredOverlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,T filter,TraceResult *res,unsigned int flags)
{
	return Overlap(geometry,origin,rot,PxFilteredQueryFilterCallback(filter),res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,physx::PxQueryFilterCallback &filter,TraceResult *res,unsigned int flags)
{
	physx::PxTransform t;
	t.p = physx::PxVec3(origin.x,origin.y,origin.z);
	t.q = physx::PxQuat(rot.x,rot.y,rot.z,rot.w);
	physx::PxScene *scene = GetPhysXScene();
	physx::PxOverlapHit *touches = NULL;
	physx::PxQueryFilterData fd;
	InitializeFlags(flags,fd.flags);
	bool bAnyHit = ((fd.flags &physx::PxQueryFlag::eANY_HIT) == physx::PxQueryFlag::eANY_HIT) ? true : false;
	unsigned int numTouches = 0;
	if(bAnyHit == false && (flags &FTRACE_TOUCHING_HITS) == FTRACE_TOUCHING_HITS)
	{
		numTouches = 64;
		touches = new physx::PxOverlapHit[numTouches];
	}
	physx::PxOverlapBuffer hit(touches,numTouches);
	bool b = scene->overlap(
		geometry,
		t,
		hit,
		fd,
		&filter
	);
	if(res != NULL)
		FillOverlapResult(res,hit);
	if(res == NULL || b == false || bAnyHit == true)
	{
		if(touches != NULL)
			delete[] touches;
		return b;
	}
	if(touches != NULL)
		delete[] touches;
	return true;
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,BaseEntity *filter,TraceResult *res,unsigned int flags)
{
	if(filter == NULL)
		return Overlap(geometry,origin,rot,res,flags);
	return FilteredOverlap<BaseEntity*>(geometry,origin,rot,filter,res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,EntityHandle &hFilter,TraceResult *res,unsigned int flags)
{
	return Overlap(geometry,origin,rot,hFilter.get(),res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,std::vector<BaseEntity*> &filter,TraceResult *res,unsigned int flags)
{
	return FilteredOverlap<std::vector<BaseEntity*>&>(geometry,origin,rot,filter,res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,physx::PxRigidActor *filter,TraceResult *res,unsigned int flags)
{
	return FilteredOverlap<physx::PxRigidActor*>(geometry,origin,rot,filter,res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,std::vector<physx::PxRigidActor*> &filter,TraceResult *res,unsigned int flags)
{
	return FilteredOverlap<std::vector<physx::PxRigidActor*>&>(geometry,origin,rot,filter,res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),TraceResult *res,unsigned int flags)
{
	return Overlap(geometry,origin,rot,PxFilteredQueryFilterFunctionCallback(preFilter),res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags)
{
	return Overlap(geometry,origin,rot,PxFilteredQueryFilterFunctionCallback(postFilter),res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags)
{
	return Overlap(geometry,origin,rot,PxFilteredQueryFilterFunctionCallback(preFilter,postFilter),res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,physx::PxQueryHitType::Enum(*filter)(BaseEntity*),TraceResult *res,unsigned int flags)
{
	return Overlap(geometry,origin,rot,PxFilteredQueryFilterEntityCallback(filter),res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,physx::PxQueryHitType::Enum(*filter)(const physx::PxRigidActor*),TraceResult *res,unsigned int flags)
{
	return Overlap(geometry,origin,rot,PxFilteredQueryFilterActorCallback(filter),res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,Quat &rot,physx::PxQueryHitType::Enum(*filter)(PhysObj*),TraceResult *res,unsigned int flags)
{
	return Overlap(geometry,origin,rot,PxFilteredQueryFilterPhysObjCallback(filter),res,flags);
}

bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,BaseEntity *filter,TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),filter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,EntityHandle &hFilter,TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),hFilter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,std::vector<BaseEntity*> &filter,TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),filter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,physx::PxRigidActor *filter,TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),filter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,std::vector<physx::PxRigidActor*> &filter,TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),filter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,physx::PxQueryFilterCallback &filter,TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),filter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),preFilter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),postFilter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,physx::PxQueryHitType::Enum(*preFilter)(const physx::PxFilterData&,const physx::PxShape*,const physx::PxRigidActor*,physx::PxHitFlags&),physx::PxQueryHitType::Enum(*postFilter)(const physx::PxFilterData&,const physx::PxQueryHit&),TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),preFilter,postFilter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,physx::PxQueryHitType::Enum(*filter)(BaseEntity*),TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),filter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,physx::PxQueryHitType::Enum(*filter)(const physx::PxRigidActor*),TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),filter,res,flags);}
bool Game::Overlap(const physx::PxGeometry &geometry,Vector3 &origin,physx::PxQueryHitType::Enum(*filter)(PhysObj*),TraceResult *res,unsigned int flags) {return Overlap(geometry,origin,uquat::identity(),filter,res,flags);}
#endif
