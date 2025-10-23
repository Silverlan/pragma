// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <memory>
#include <string>

module pragma.shared;

import :physics.raycast;

void TraceResult::InitializeMeshes()
{
	if(meshInfo != nullptr)
		return;
	meshInfo = std::make_shared<MeshInfo>();
	if(entity.valid() == false)
		return;
	auto mdlComponent = entity->GetModelComponent();
	auto hMdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return;
	hMdl->GetBodyGroupMeshes(mdlComponent->GetBodyGroups(), 0u, meshInfo->meshes);
	auto closestDist = std::numeric_limits<float>::max();

	auto pTrComponent = entity->GetTransformComponent();
	auto origin = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	auto rot = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();

	auto startPosLocal = startPosition;
	uvec::world_to_local(origin, rot, startPosLocal);

	auto maxDist = distance + 50.f;
	auto dir = position - startPosition;
	uvec::normalize(&dir);
	uvec::world_to_local(origin, rot, dir);
	dir *= maxDist;

	Intersection::LineMeshResult res {};
	for(auto &mesh : meshInfo->meshes) {
		Vector3 min, max;
		mesh->GetBounds(min, max);
		auto dist = umath::min(maxDist, static_cast<float>(res.hitValue));
		auto t = 0.f;
		if(!umath::intersection::point_in_aabb(startPosLocal, min, max) && (umath::intersection::line_aabb(startPosLocal, dir, min, max, &t) != umath::intersection::Result::Intersect || umath::abs(t) > (dist / maxDist)))
			continue;
		auto &subMeshes = mesh->GetSubMeshes();
		for(auto &subMesh : subMeshes) {
			subMesh->GetBounds(min, max);
			if(!umath::intersection::point_in_aabb(startPosLocal, min, max) && (umath::intersection::line_aabb(startPosLocal, dir, min, max, &t) != umath::intersection::Result::Intersect || umath::abs(t) > (dist / maxDist)))
				continue;
			if(Intersection::LineMesh(startPosLocal, dir, *subMesh, res, true, nullptr, nullptr) == false || umath::abs(res.hitValue) > (dist / maxDist))
				continue;
			meshInfo->mesh = mesh.get();
			meshInfo->subMesh = subMesh.get();
		}
	}
}

void TraceResult::GetMeshes(ModelMesh **outMesh, pragma::ModelSubMesh **outSubMesh)
{
	InitializeMeshes();
	*outMesh = meshInfo->mesh;
	*outSubMesh = meshInfo->subMesh;
}
msys::Material *TraceResult::GetMaterial()
{
	if(entity.valid() == false)
		return nullptr;
	auto &hMdl = entity->GetModel();
	if(hMdl == nullptr)
		return nullptr;
	ModelMesh *mesh = nullptr;
	pragma::ModelSubMesh *subMesh = nullptr;
	GetMeshes(&mesh, &subMesh);
	if(mesh == nullptr || subMesh == nullptr)
		return nullptr;
	auto texId = hMdl->GetMaterialIndex(*subMesh, entity->GetSkin());
	auto &materials = hMdl->GetMaterials();
	if(texId.has_value() == false || *texId >= materials.size())
		return nullptr;
	return materials.at(*texId).get();
}
bool TraceResult::GetMaterial(std::string &mat)
{
	if(entity.valid() == false)
		return false;
	auto &hMdl = entity->GetModel();
	if(hMdl == nullptr)
		return false;
	ModelMesh *mesh = nullptr;
	pragma::ModelSubMesh *subMesh = nullptr;
	GetMeshes(&mesh, &subMesh);
	if(mesh == nullptr || subMesh == nullptr)
		return false;
	auto texId = hMdl->GetMaterialIndex(*subMesh, entity->GetSkin());
	auto *pMat = texId.has_value() ? hMdl->GetMaterial(*texId) : nullptr;
	if(pMat == nullptr)
		return false;
	mat = pMat->GetName();
	return true;
}

/////////////////////////////////////////

TraceData::TraceData() : m_flags(pragma::physics::RayCastFlags::Default), m_filterMask(pragma::physics::CollisionMask::All), m_filterGroup(pragma::physics::CollisionMask::Default)
{
	m_tStart.SetIdentity();
	m_tEnd.SetIdentity();
}
TraceData::TraceData(const TraceData &other) : m_tStart(other.m_tStart), m_tEnd(other.m_tEnd), m_flags(other.m_flags), m_bHasTarget(other.m_bHasTarget), m_shape {other.m_shape}, m_filter {other.m_filter}, m_filterMask(other.m_filterMask), m_filterGroup(other.m_filterGroup) {}
TraceData::~TraceData() {}
void TraceData::SetShape(const pragma::physics::IConvexShape &shape) { m_shape = std::dynamic_pointer_cast<pragma::physics::IConvexShape>(const_cast<pragma::physics::IConvexShape &>(shape).shared_from_this()); }
const pragma::physics::IConvexShape *TraceData::GetShape() const { return m_shape.get(); }
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
Vector3 TraceData::GetTargetOrigin() const { return m_tEnd.GetOrigin(); }
Quat TraceData::GetTargetRotation() const { return m_tEnd.GetRotation(); }
bool TraceData::HasTarget() const { return m_bHasTarget; }
const umath::Transform &TraceData::GetSource() const { return m_tStart; }
const umath::Transform &TraceData::GetTarget() const { return m_tEnd; }
pragma::physics::CollisionMask TraceData::GetCollisionFilterMask() const { return m_filterMask; }
pragma::physics::CollisionMask TraceData::GetCollisionFilterGroup() const { return m_filterGroup; }
void TraceData::SetSource(const Vector3 &origin) { m_tStart.SetOrigin(origin); }
void TraceData::SetSourceRotation(const Quat &rot) { m_tStart.SetRotation(rot); }
void TraceData::SetSource(const umath::Transform &t) { m_tStart = t; }
void TraceData::SetTarget(const Vector3 &target)
{
	m_tEnd.SetOrigin(target);
	m_bHasTarget = true;
}
void TraceData::SetTargetRotation(const Quat &rot)
{
	m_tEnd.SetRotation(rot);
	m_bHasTarget = true;
}
void TraceData::SetTarget(const umath::Transform &t)
{
	m_tEnd = t;
	m_bHasTarget = true;
}
void TraceData::SetRotation(const Quat &rot)
{
	SetSourceRotation(rot);
	SetTargetRotation(rot);
}
void TraceData::SetFlags(pragma::physics::RayCastFlags flags) { m_flags = flags; }
pragma::physics::RayCastFlags TraceData::GetFlags() const { return m_flags; }
void TraceData::SetCollisionFilterMask(pragma::physics::CollisionMask mask) { m_filterMask = mask; }
void TraceData::SetCollisionFilterGroup(pragma::physics::CollisionMask group) { m_filterGroup = group; }
void TraceData::SetFilter(const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &filter) { m_filter = filter; }
void TraceData::SetFilter(pragma::ecs::BaseEntity &ent) { SetFilter(std::make_unique<::pragma::physics::EntityRayCastFilterCallback>(ent)); }
void TraceData::SetFilter(std::vector<EntityHandle> &&ents) { SetFilter(std::make_unique<::pragma::physics::MultiEntityRayCastFilterCallback>(std::move(ents))); }
void TraceData::SetFilter(pragma::physics::PhysObj &phys) { SetFilter(std::make_unique<::pragma::physics::PhysObjRayCastFilterCallback>(phys)); }
void TraceData::SetFilter(pragma::physics::ICollisionObject &colObj) { SetFilter(std::make_unique<::pragma::physics::CollisionObjRayCastFilterCallback>(colObj)); }
void TraceData::SetFilter(const std::function<pragma::physics::RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &preFilter, const std::function<pragma::physics::RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &postFilter)
{
	SetFilter(std::make_unique<::pragma::physics::CustomRayCastFilterCallback>(preFilter, postFilter));
}
const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &TraceData::GetFilter() const { return m_filter; }
Vector3 TraceData::GetDirection() const
{
	auto src = GetSourceOrigin();
	auto tgt = GetTargetOrigin();
	auto dir = tgt - src;
	uvec::normalize(&dir);
	return dir;
}
float TraceData::GetDistance() const
{
	auto src = GetSourceOrigin();
	auto tgt = GetTargetOrigin();
	return uvec::distance(src, tgt);
}
bool TraceData::HasFlag(pragma::physics::RayCastFlags flag) const { return ((UInt32(m_flags) & UInt32(flag)) != 0) ? true : false; }

TraceResult::TraceResult(const TraceData &data) : fraction {1.f}, position {data.GetTargetOrigin()}, startPosition {data.GetSourceOrigin()} {}
TraceResult::~TraceResult() {}
