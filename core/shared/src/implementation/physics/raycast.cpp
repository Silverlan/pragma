// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.raycast;

void pragma::physics::TraceResult::InitializeMeshes()
{
	if(meshInfo != nullptr)
		return;
	meshInfo = pragma::util::make_shared<MeshInfo>();
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

	math::intersection::LineMeshResult res {};
	for(auto &mesh : meshInfo->meshes) {
		Vector3 min, max;
		mesh->GetBounds(min, max);
		auto dist = math::min(maxDist, static_cast<float>(res.hitValue));
		auto t = 0.f;
		if(!math::intersection::point_in_aabb(startPosLocal, min, max) && (math::intersection::line_aabb(startPosLocal, dir, min, max, &t) != math::intersection::Result::Intersect || math::abs(t) > (dist / maxDist)))
			continue;
		auto &subMeshes = mesh->GetSubMeshes();
		for(auto &subMesh : subMeshes) {
			subMesh->GetBounds(min, max);
			if(!math::intersection::point_in_aabb(startPosLocal, min, max) && (math::intersection::line_aabb(startPosLocal, dir, min, max, &t) != math::intersection::Result::Intersect || math::abs(t) > (dist / maxDist)))
				continue;
			if(pragma::math::intersection::line_with_mesh(startPosLocal, dir, *subMesh, res, true, nullptr, nullptr) == false || math::abs(res.hitValue) > (dist / maxDist))
				continue;
			meshInfo->mesh = mesh.get();
			meshInfo->subMesh = subMesh.get();
		}
	}
}

void pragma::physics::TraceResult::GetMeshes(geometry::ModelMesh **outMesh, geometry::ModelSubMesh **outSubMesh)
{
	InitializeMeshes();
	*outMesh = meshInfo->mesh;
	*outSubMesh = meshInfo->subMesh;
}
pragma::material::Material *pragma::physics::TraceResult::GetMaterial()
{
	if(entity.valid() == false)
		return nullptr;
	auto &hMdl = entity->GetModel();
	if(hMdl == nullptr)
		return nullptr;
	geometry::ModelMesh *mesh = nullptr;
	geometry::ModelSubMesh *subMesh = nullptr;
	GetMeshes(&mesh, &subMesh);
	if(mesh == nullptr || subMesh == nullptr)
		return nullptr;
	auto texId = hMdl->GetMaterialIndex(*subMesh, entity->GetSkin());
	auto &materials = hMdl->GetMaterials();
	if(texId.has_value() == false || *texId >= materials.size())
		return nullptr;
	return materials.at(*texId).get();
}
bool pragma::physics::TraceResult::GetMaterial(std::string &mat)
{
	if(entity.valid() == false)
		return false;
	auto &hMdl = entity->GetModel();
	if(hMdl == nullptr)
		return false;
	geometry::ModelMesh *mesh = nullptr;
	geometry::ModelSubMesh *subMesh = nullptr;
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

pragma::physics::TraceData::TraceData() : m_flags(RayCastFlags::Default), m_filterMask(CollisionMask::All), m_filterGroup(CollisionMask::Default)
{
	m_tStart.SetIdentity();
	m_tEnd.SetIdentity();
}
pragma::physics::TraceData::TraceData(const TraceData &other) : m_tStart(other.m_tStart), m_tEnd(other.m_tEnd), m_flags(other.m_flags), m_bHasTarget(other.m_bHasTarget), m_shape {other.m_shape}, m_filter {other.m_filter}, m_filterMask(other.m_filterMask), m_filterGroup(other.m_filterGroup) {}
pragma::physics::TraceData::~TraceData() {}
void pragma::physics::TraceData::SetShape(const IConvexShape &shape) { m_shape = std::dynamic_pointer_cast<IConvexShape>(const_cast<IConvexShape &>(shape).shared_from_this()); }
const pragma::physics::IConvexShape *pragma::physics::TraceData::GetShape() const { return m_shape.get(); }
Vector3 pragma::physics::TraceData::GetSourceOrigin() const
{
	auto &t = GetSource();
	return t.GetOrigin();
}
Quat pragma::physics::TraceData::GetSourceRotation() const
{
	auto &t = GetSource();
	return t.GetRotation();
}
Vector3 pragma::physics::TraceData::GetTargetOrigin() const { return m_tEnd.GetOrigin(); }
Quat pragma::physics::TraceData::GetTargetRotation() const { return m_tEnd.GetRotation(); }
bool pragma::physics::TraceData::HasTarget() const { return m_bHasTarget; }
const pragma::math::Transform &pragma::physics::TraceData::GetSource() const { return m_tStart; }
const pragma::math::Transform &pragma::physics::TraceData::GetTarget() const { return m_tEnd; }
pragma::physics::CollisionMask pragma::physics::TraceData::GetCollisionFilterMask() const { return m_filterMask; }
pragma::physics::CollisionMask pragma::physics::TraceData::GetCollisionFilterGroup() const { return m_filterGroup; }
void pragma::physics::TraceData::SetSource(const Vector3 &origin) { m_tStart.SetOrigin(origin); }
void pragma::physics::TraceData::SetSourceRotation(const Quat &rot) { m_tStart.SetRotation(rot); }
void pragma::physics::TraceData::SetSource(const math::Transform &t) { m_tStart = t; }
void pragma::physics::TraceData::SetTarget(const Vector3 &target)
{
	m_tEnd.SetOrigin(target);
	m_bHasTarget = true;
}
void pragma::physics::TraceData::SetTargetRotation(const Quat &rot)
{
	m_tEnd.SetRotation(rot);
	m_bHasTarget = true;
}
void pragma::physics::TraceData::SetTarget(const math::Transform &t)
{
	m_tEnd = t;
	m_bHasTarget = true;
}
void pragma::physics::TraceData::SetRotation(const Quat &rot)
{
	SetSourceRotation(rot);
	SetTargetRotation(rot);
}
void pragma::physics::TraceData::SetFlags(RayCastFlags flags) { m_flags = flags; }
pragma::physics::RayCastFlags pragma::physics::TraceData::GetFlags() const { return m_flags; }
void pragma::physics::TraceData::SetCollisionFilterMask(CollisionMask mask) { m_filterMask = mask; }
void pragma::physics::TraceData::SetCollisionFilterGroup(CollisionMask group) { m_filterGroup = group; }
void pragma::physics::TraceData::SetFilter(const std::shared_ptr<IRayCastFilterCallback> &filter) { m_filter = filter; }
void pragma::physics::TraceData::SetFilter(ecs::BaseEntity &ent) { SetFilter(std::make_unique<EntityRayCastFilterCallback>(ent)); }
void pragma::physics::TraceData::SetFilter(std::vector<EntityHandle> &&ents) { SetFilter(std::make_unique<MultiEntityRayCastFilterCallback>(std::move(ents))); }
void pragma::physics::TraceData::SetFilter(PhysObj &phys) { SetFilter(std::make_unique<PhysObjRayCastFilterCallback>(phys)); }
void pragma::physics::TraceData::SetFilter(ICollisionObject &colObj) { SetFilter(std::make_unique<CollisionObjRayCastFilterCallback>(colObj)); }
void pragma::physics::TraceData::SetFilter(const std::function<RayCastHitType(IShape &, IRigidBody &)> &preFilter, const std::function<RayCastHitType(IShape &, IRigidBody &)> &postFilter)
{
	SetFilter(std::make_unique<CustomRayCastFilterCallback>(preFilter, postFilter));
}
const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &pragma::physics::TraceData::GetFilter() const { return m_filter; }
Vector3 pragma::physics::TraceData::GetDirection() const
{
	auto src = GetSourceOrigin();
	auto tgt = GetTargetOrigin();
	auto dir = tgt - src;
	uvec::normalize(&dir);
	return dir;
}
float pragma::physics::TraceData::GetDistance() const
{
	auto src = GetSourceOrigin();
	auto tgt = GetTargetOrigin();
	return uvec::distance(src, tgt);
}
bool pragma::physics::TraceData::HasFlag(RayCastFlags flag) const { return ((UInt32(m_flags) & UInt32(flag)) != 0) ? true : false; }

pragma::physics::TraceResult::TraceResult(const TraceData &data) : fraction {1.f}, position {data.GetTargetOrigin()}, startPosition {data.GetSourceOrigin()} {}
pragma::physics::TraceResult::~TraceResult() {}
