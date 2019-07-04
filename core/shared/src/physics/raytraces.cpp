#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/physics/raytraces.h"
#include "pragma/physics/collisiontypes.h"
#include <mathutil/color.h>
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/collision_object.hpp"
#include "pragma/physics/shape.hpp"
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

	Intersection::LineMeshResult res{};
	for(auto &mesh : m_meshInfo->meshes)
	{
		Vector3 min,max;
		mesh->GetBounds(min,max);
		auto dist = umath::min(maxDist,static_cast<float>(res.hitValue));
		auto t = 0.f;
		if(Intersection::LineAABB(startPosLocal,dir,min,max,&t) != Intersection::Result::Intersect || umath::abs(t) > (dist /maxDist))
			continue;
		auto &subMeshes = mesh->GetSubMeshes();
		for(auto &subMesh : subMeshes)
		{
			subMesh->GetBounds(min,max);
			if(Intersection::LineAABB(startPosLocal,dir,min,max,&t) != Intersection::Result::Intersect || umath::abs(t) > (dist /maxDist))
				continue;
			if(Intersection::LineMesh(startPosLocal,dir,*subMesh,res,true,nullptr,nullptr) == false || umath::abs(res.hitValue) > (dist /maxDist))
				continue;
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
	: m_flags(RayCastFlags::Default),m_filterMask(CollisionMask::All),m_filterGroup(CollisionMask::Default)
{
	m_tStart.SetIdentity();
	m_tEnd.SetIdentity();
}
TraceData::TraceData(const TraceData &other)
	: m_tStart(other.m_tStart),m_tEnd(other.m_tEnd),
	m_flags(other.m_flags),m_bHasTarget(other.m_bHasTarget),
	m_shape{other.m_shape},
	m_filterMask(other.m_filterMask),m_filterGroup(other.m_filterGroup)
#ifdef ENABLE_DEPRECATED_PHYSICS
	,m_filter(std::shared_ptr<TraceFilterBase>(other.m_filter->Copy()))
#endif
{}
void TraceData::SetShape(const pragma::physics::IConvexShape &shape)
{
	m_shape = util::weak_shared_handle_cast<pragma::physics::IBase,pragma::physics::IConvexShape>(shape.GetHandle());
}
const pragma::physics::IConvexShape *TraceData::GetShape() const {return m_shape.Get();}
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
const pragma::physics::Transform &TraceData::GetSource() const
{
	return m_tStart;
}
const pragma::physics::Transform &TraceData::GetTarget() const {return m_tEnd;}
CollisionMask TraceData::GetCollisionFilterMask() const {return m_filterMask;}
CollisionMask TraceData::GetCollisionFilterGroup() const {return m_filterGroup;}
void TraceData::SetSource(const Vector3 &origin) {m_tStart.SetOrigin(origin);}
void TraceData::SetSourceRotation(const Quat &rot) {m_tStart.SetRotation(rot);}
void TraceData::SetSource(const pragma::physics::Transform &t) {m_tStart = t;}
void TraceData::SetTarget(const Vector3 &target) {m_tEnd.SetOrigin(target);m_bHasTarget = true;}
void TraceData::SetTargetRotation(const Quat &rot) {m_tEnd.SetRotation(rot);m_bHasTarget = true;}
void TraceData::SetTarget(const pragma::physics::Transform &t) {m_tEnd = t;m_bHasTarget = true;}
void TraceData::SetRotation(const Quat &rot)
{
	SetSourceRotation(rot);
	SetTargetRotation(rot);
}
void TraceData::SetFlags(RayCastFlags flags) {m_flags = flags;}
RayCastFlags TraceData::GetFlags() const {return m_flags;}
#ifdef ENABLE_DEPRECATED_PHYSICS
const TraceFilterBase *TraceData::GetFilter() const {return m_filter.get();}
#endif
void TraceData::SetCollisionFilterMask(CollisionMask mask) {m_filterMask = mask;}
void TraceData::SetCollisionFilterGroup(CollisionMask group) {m_filterGroup = group;}
void TraceData::SetFilter(const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &filter) {m_filter = filter;}
const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &TraceData::GetFilter() const {return m_filter;}
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
bool TraceData::HasFlag(RayCastFlags flag) const {return ((UInt32(m_flags) &UInt32(flag)) != 0) ? true : false;}

TraceResult::TraceResult(const TraceData &data)
	: fraction{1.f},position{data.GetTargetOrigin()},startPosition{data.GetSourceOrigin()}
{}
