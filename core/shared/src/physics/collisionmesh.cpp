/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/collisionmesh.h"
#include <mathutil/uvec.h>
#include <pragma/math/intersection.h>
#include "pragma/physics/shape.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/model/modelmesh.h"
#include "pragma/model/model.h"
#include <udm.hpp>

std::shared_ptr<CollisionMesh> CollisionMesh::Create(Game *game) { return std::shared_ptr<CollisionMesh>(new CollisionMesh(game)); }
std::shared_ptr<CollisionMesh> CollisionMesh::Create(const CollisionMesh &other) { return std::shared_ptr<CollisionMesh>(new CollisionMesh(other)); }
std::shared_ptr<CollisionMesh> CollisionMesh::Load(Game &game, Model &mdl, const udm::AssetData &data, std::string &outErr)
{
	auto mesh = Create(&game);
	auto result = mesh->LoadFromAssetData(game, mdl, data, outErr);
	return result ? mesh : nullptr;
}
CollisionMesh::SoftBodyInfo::SoftBodyInfo() { info = std::make_shared<PhysSoftBodyInfo>(); }
bool CollisionMesh::SoftBodyInfo::operator==(const SoftBodyInfo &other) const
{
	static_assert(sizeof(SoftBodyInfo) == 80, "Update this function when making changes to this class!");
	if(!(triangles == other.triangles && anchors == other.anchors && static_cast<bool>(info) == static_cast<bool>(other.info)))
		return false;
	if(info && *info != *other.info)
		return false;
	return true;
}
bool CollisionMesh::SoftBodyAnchor::operator==(const SoftBodyAnchor &other) const
{
	static_assert(sizeof(SoftBodyAnchor) == 11, "Update this function when making changes to this class!");
	return vertexIndex == other.vertexIndex && boneId == other.boneId && influence == other.influence && flags == other.flags;
}
CollisionMesh::CollisionMesh(Game *game) : std::enable_shared_from_this<CollisionMesh>(), m_game(game), m_uuid {util::generate_uuid_v4()} {}
CollisionMesh::CollisionMesh(const CollisionMesh &other)
{
	m_game = other.m_game;
	m_vertices = other.m_vertices;
	m_triangles = other.m_triangles;
	m_surfaceMaterials = other.m_surfaceMaterials;
	m_min = other.m_min;
	m_max = other.m_max;
	m_origin = other.m_origin;
	m_shape = other.m_shape;
	m_bConvex = other.m_bConvex;
	m_boneID = other.m_boneID;
	m_surfaceMaterialId = other.m_surfaceMaterialId;
	m_centerOfMass = other.m_centerOfMass;
	m_volume = other.m_volume;
	m_mass = other.m_mass;
	m_softBodyInfo = (m_softBodyInfo != nullptr) ? std::make_shared<SoftBodyInfo>(*other.m_softBodyInfo) : nullptr;
	static_assert(sizeof(CollisionMesh) == 216, "Update this function when making changes to this class!");
}
bool CollisionMesh::operator==(const CollisionMesh &other) const
{
	if(m_vertices.size() != other.m_vertices.size())
		return false;
	if(!(m_vertices == other.m_vertices && m_triangles == other.m_triangles && m_surfaceMaterials == other.m_surfaceMaterials && uvec::cmp(m_min, other.m_min) && uvec::cmp(m_max, other.m_max) && uvec::cmp(m_origin, other.m_origin) &&
	     // m_shape == other.m_shape &&
	     m_bConvex == other.m_bConvex && m_boneID == other.m_boneID && m_surfaceMaterialId == other.m_surfaceMaterialId && uvec::cmp(m_centerOfMass, other.m_centerOfMass) && m_volume == other.m_volume && m_mass == other.m_mass
	     && ((m_softBodyInfo == nullptr && other.m_softBodyInfo == nullptr) || *m_softBodyInfo == *other.m_softBodyInfo)))
		return false;
	for(auto i = decltype(m_vertices.size()) {0u}; i < m_vertices.size(); ++i) {
		if(uvec::cmp(m_vertices[i], other.m_vertices[i]) == false)
			return false;
	}
	static_assert(sizeof(CollisionMesh) == 216, "Update this function when making changes to this class!");
	return true;
}
void CollisionMesh::SetMass(float mass) { m_mass = mass; }
float CollisionMesh::GetMass() const { return m_mass; }
int CollisionMesh::GetSurfaceMaterial() const { return m_surfaceMaterialId; }
void CollisionMesh::SetSurfaceMaterial(int id) { m_surfaceMaterialId = id; }
void CollisionMesh::SetSurfaceMaterial(const std::string &surfMat)
{
	m_surfaceMaterialId = 0;
	auto *mat = m_game->GetSurfaceMaterial(surfMat);
	if(mat == nullptr)
		return;
	SetSurfaceMaterial(static_cast<int32_t>(mat->GetIndex()));
}
std::vector<int> &CollisionMesh::GetSurfaceMaterials() { return m_surfaceMaterials; }
const util::Uuid &CollisionMesh::GetUuid() const { return m_uuid; }
void CollisionMesh::SetUuid(const util::Uuid &uuid) { m_uuid = uuid; }
void CollisionMesh::SetConvex(bool bConvex) { m_bConvex = bConvex; }
bool CollisionMesh::IsConvex() const { return m_bConvex; }
void CollisionMesh::AddVertex(const Vector3 &v)
{
	if(m_vertices.size() == m_vertices.capacity())
		m_vertices.reserve(static_cast<uint32_t>(m_vertices.size() * 1.5));
	m_vertices.push_back(v);
}
void CollisionMesh::Rotate(const Quat &rot)
{
	for(auto &v : m_vertices)
		uvec::rotate(&v, rot);
	uvec::rotate(&m_origin, rot);
	uvec::rotate(&m_min, rot);
	uvec::rotate(&m_max, rot);
}
void CollisionMesh::Translate(const Vector3 &t)
{
	for(auto &v : m_vertices)
		v += t;
	m_origin += t;
	m_min += t;
	m_max += t;
}
void CollisionMesh::Scale(const Vector3 &scale)
{
	for(auto &v : m_vertices)
		v *= scale;
	m_origin *= scale;
	m_min *= scale;
	m_max *= scale;
}
void CollisionMesh::Mirror(pragma::Axis axis)
{
	auto transform = pragma::model::get_mirror_transform_vector(axis);
	for(auto &v : m_vertices)
		v *= transform;
	for(size_t i = 0; i < m_triangles.size(); i += 3)
		umath::swap(m_triangles[i], m_triangles[i + 1]);
	m_min *= transform;
	m_max *= transform;
	uvec::to_min_max(m_min, m_max);

	m_origin *= transform;
	m_centerOfMass *= transform;
}
std::shared_ptr<pragma::physics::IShape> CollisionMesh::CreateShape(const Vector3 &scale) const
{
	auto *physEnv = m_game->GetPhysicsEnvironment();
	if(IsSoftBody() || physEnv == nullptr)
		return nullptr;
	auto &materials = *m_game->GetSurfaceMaterials();
	auto bConvex = IsConvex();
	std::shared_ptr<pragma::physics::IShape> shape = nullptr;
	auto bScale = (scale != Vector3 {1.f, 1.f, 1.f}) ? true : false;

	pragma::physics::IMaterial *mat = nullptr;
	if(materials.empty())
		mat = &physEnv->GetGenericMaterial();
	else
		mat = &materials.front().GetPhysicsMaterial();
	if(bConvex == true) {
		shape = physEnv->CreateConvexHullShape(*mat);
		if(shape == nullptr)
			return nullptr;
		auto *ptrShape = shape->GetConvexHullShape();
		ptrShape->SetCollisionMesh(*const_cast<CollisionMesh *>(this));
		ptrShape->SetSurfaceMaterial(GetSurfaceMaterial());
		ptrShape->SetLocalScaling(Vector3(1.f, 1.f, 1.f));
		ptrShape->ReservePoints(m_vertices.size());
		for(unsigned int i = 0; i < m_vertices.size(); i++) {
			auto &v = m_vertices[i];
			if(bScale == false)
				ptrShape->AddPoint(v);
			else
				ptrShape->AddPoint(v * scale);
		}
		ptrShape->ReserveTriangles(m_triangles.size() / 3);
		for(auto i = decltype(m_triangles.size()) {0u}; i < m_triangles.size(); i += 3)
			ptrShape->AddTriangle(m_triangles.at(i), m_triangles.at(i + 1), m_triangles.at(i + 2));
		ptrShape->Build();
	}
	else {
		shape = physEnv->CreateTriangleShape(*mat);
		if(shape == nullptr)
			return nullptr;
		auto *ptrShape = shape->GetTriangleShape();
		auto numMats = m_surfaceMaterials.size();
		auto &tris = GetTriangles();
		if(tris.empty() == false) {
			assert((m_vertices.size() % 3) == 0);
			auto numTris = tris.size() / 3;
			ptrShape->ReserveTriangles(numTris);
			for(auto i = decltype(numTris) {0u}; i < (numTris * 3); i += 3) {
				auto triId = i / 3;
				const SurfaceMaterial *mat = nullptr;
				auto matId = 0;
				if(triId < numMats)
					matId = m_surfaceMaterials[triId];
				mat = &materials[matId];
				auto &a = m_vertices.at(tris.at(i));
				auto &b = m_vertices.at(tris.at(i + 1));
				auto &c = m_vertices.at(tris.at(i + 2));
				if(bScale == false)
					ptrShape->AddTriangle(a, b, c, mat);
				else
					ptrShape->AddTriangle(a * scale, b * scale, c * scale, mat);
			}
		}
		else {
			// Assume that the vertices are making up a triangle mesh
			assert((m_vertices.size() % 3) == 0);
			auto numTris = m_vertices.size() / 3;
			ptrShape->ReserveTriangles(m_vertices.size() / 3);
			for(auto i = decltype(numTris) {0u}; i < (numTris * 3); i += 3) {
				auto triId = i / 3;
				const SurfaceMaterial *mat = nullptr;
				auto matId = 0;
				if(triId < numMats)
					matId = m_surfaceMaterials[triId];
				mat = &materials[matId];
				auto &a = m_vertices[i];
				auto &b = m_vertices[i + 1];
				auto &c = m_vertices[i + 2];
				if(bScale == false)
					ptrShape->AddTriangle(a, b, c, mat);
				else
					ptrShape->AddTriangle(a * scale, b * scale, c * scale, mat);
			}
		}
		ptrShape->Build(&materials);
	}
	if(shape) {
		shape->SetMass(GetMass());
		shape->SetLocalPose(umath::Transform {-GetOrigin(), uquat::identity()});
	}
	return shape;
}
void CollisionMesh::ClearShape() { m_shape = nullptr; }
void CollisionMesh::UpdateShape()
{
	ClearShape();
	if(m_vertices.empty() == true)
		return;
	m_shape = CreateShape();
}
void CollisionMesh::SetBoneParent(int boneID) { m_boneID = boneID; }
int CollisionMesh::GetBoneParent() const { return m_boneID; }
void CollisionMesh::SetOrigin(const Vector3 &origin) { m_origin = origin; }
const Vector3 &CollisionMesh::GetOrigin() const { return const_cast<CollisionMesh *>(this)->GetOrigin(); }
Vector3 &CollisionMesh::GetOrigin() { return m_origin; }
std::vector<Vector3> &CollisionMesh::GetVertices() { return m_vertices; }
void CollisionMesh::Validate()
{
	Vector3 min, max;
	GetAABB(&min, &max);
	pragma::model::validate_value(min);
	pragma::model::validate_value(max);
	pragma::model::validate_value(GetCenterOfMass());
	pragma::model::validate_value(GetMass());
	pragma::model::validate_value(GetOrigin());
	pragma::model::validate_value(GetVolume());
	for(auto &v : GetVertices())
		pragma::model::validate_value(v);
}
void CollisionMesh::CalculateBounds()
{
	auto numVerts = m_vertices.size();
	if(numVerts == 0)
		return;
	m_min = m_vertices[0];
	m_max = m_vertices[0];
	for(size_t i = 1; i < numVerts; i++) {
		uvec::min(&m_min, m_vertices[i]);
		uvec::max(&m_max, m_vertices[i]);
	}
}
void CollisionMesh::Update(ModelUpdateFlags flags)
{
	if((flags & ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None)
		CalculateBounds();
	if((flags & ModelUpdateFlags::InitializeCollisionShapes) != ModelUpdateFlags::None)
		UpdateShape(); // TODO: Surface materials?
}
void CollisionMesh::Centralize()
{
	Vector3 center {};
	for(auto &v : m_vertices)
		center += v;
	if(m_vertices.empty() == false)
		center /= static_cast<float>(m_vertices.size());
	SetOrigin(GetOrigin() + -center);
	for(auto &v : m_vertices)
		v -= center;
}
void CollisionMesh::GetAABB(Vector3 *min, Vector3 *max) const
{
	*min = m_min;
	*max = m_max;
}
void CollisionMesh::SetAABB(Vector3 &min, Vector3 &max)
{
	m_min = min;
	m_max = max;
}
std::shared_ptr<pragma::physics::IShape> CollisionMesh::GetShape() { return m_shape; }
bool CollisionMesh::IntersectAABB(Vector3 *min, Vector3 *max)
{
	if(umath::intersection::aabb_aabb(m_min, m_max, *min, *max) == umath::intersection::Intersect::Outside)
		return false;
	for(int i = 0; i < m_vertices.size(); i += 3) {
		if(umath::intersection::aabb_triangle(m_min, m_max, m_vertices[i], m_vertices[i + 1], m_vertices[i + 2]))
			return true;
	}
	// TODO: Check if bounds are WITHIN mesh (Before checking triangles?)
	return false;
}

const std::vector<uint16_t> &CollisionMesh::GetTriangles() const { return const_cast<CollisionMesh *>(this)->GetTriangles(); }
std::vector<uint16_t> &CollisionMesh::GetTriangles() { return m_triangles; }
void CollisionMesh::CalculateVolumeAndCom() { m_volume = umath::geometry::calc_volume_of_polyhedron(m_vertices, m_triangles, &m_centerOfMass); }
const Vector3 &CollisionMesh::GetCenterOfMass() const { return m_centerOfMass; }
void CollisionMesh::SetCenterOfMass(const Vector3 &com) { m_centerOfMass = com; }
double CollisionMesh::GetVolume() const { return m_volume; }
void CollisionMesh::SetVolume(double vol) { m_volume = vol; }

void CollisionMesh::SetSoftBody(bool b)
{
	if(b == false) {
		m_softBodyInfo = nullptr;
		return;
	}
	m_softBodyInfo = std::make_shared<SoftBodyInfo>();
}
bool CollisionMesh::IsSoftBody() const { return m_softBodyInfo != nullptr; }
ModelSubMesh *CollisionMesh::GetSoftBodyMesh() const
{
	if(m_softBodyInfo == nullptr || m_softBodyInfo->subMesh.expired())
		return nullptr;
	return m_softBodyInfo->subMesh.lock().get();
}
void CollisionMesh::SetSoftBodyMesh(ModelSubMesh &mesh)
{
	if(m_softBodyInfo == nullptr)
		return;
	m_softBodyInfo->subMesh = mesh.shared_from_this();
}
const std::vector<uint32_t> *CollisionMesh::GetSoftBodyTriangles() const { return const_cast<CollisionMesh *>(this)->GetSoftBodyTriangles(); }
std::vector<uint32_t> *CollisionMesh::GetSoftBodyTriangles() { return (m_softBodyInfo != nullptr) ? &m_softBodyInfo->triangles : nullptr; }
PhysSoftBodyInfo *CollisionMesh::GetSoftBodyInfo() const { return (m_softBodyInfo != nullptr) ? m_softBodyInfo->info.get() : nullptr; }
bool CollisionMesh::AddSoftBodyAnchor(uint16_t vertIdx, uint32_t boneIdx, SoftBodyAnchor::Flags flags, float influence, uint32_t *anchorIdx)
{
	if(m_softBodyInfo == nullptr)
		return false;
	auto &anchors = m_softBodyInfo->anchors;
	if(anchors.size() == anchors.capacity())
		anchors.reserve(anchors.size() + 10);
	anchors.push_back({});
	auto &anchor = anchors.back();
	anchor.vertexIndex = vertIdx;
	anchor.boneId = boneIdx;
	anchor.flags = flags;
	anchor.influence = influence;
	if(anchorIdx != nullptr)
		*anchorIdx = anchors.size() - 1;
	return true;
}
void CollisionMesh::RemoveSoftBodyAnchor(uint32_t anchorIdx)
{
	if(m_softBodyInfo == nullptr || anchorIdx >= m_softBodyInfo->anchors.size())
		return;
	m_softBodyInfo->anchors.erase(m_softBodyInfo->anchors.begin() + anchorIdx);
}
void CollisionMesh::ClearSoftBodyAnchors()
{
	if(m_softBodyInfo == nullptr)
		return;
	m_softBodyInfo->anchors.clear();
}
const std::vector<CollisionMesh::SoftBodyAnchor> *CollisionMesh::GetSoftBodyAnchors() const { return const_cast<CollisionMesh *>(this)->GetSoftBodyAnchors(); }
std::vector<CollisionMesh::SoftBodyAnchor> *CollisionMesh::GetSoftBodyAnchors()
{
	if(m_softBodyInfo == nullptr)
		return nullptr;
	return &m_softBodyInfo->anchors;
}
bool CollisionMesh::Save(Game &game, Model &mdl, udm::AssetDataArg outData, std::string &outErr)
{
	outData.SetAssetType(PCOL_IDENTIFIER);
	outData.SetAssetVersion(PCOL_VERSION);

	auto *surfaceMaterials = game.GetSurfaceMaterials();
	auto surfMatIdx = GetSurfaceMaterial();
	auto udm = *outData;
	umath::Transform pose {};
	pose.SetOrigin(GetOrigin());
	udm["uuid"] = util::uuid_to_string(m_uuid);
	udm["bone"] = GetBoneParent();
	udm["pose"] = pose;
	if(surfMatIdx >= 0 && surfaceMaterials && surfMatIdx < surfaceMaterials->size())
		udm["surfaceMaterial"] = (*surfaceMaterials)[surfMatIdx].GetIdentifier();

	udm["bounds"]["min"] = m_min;
	udm["bounds"]["max"] = m_max;

	udm.AddArray("vertices", GetVertices(), udm::ArrayType::Compressed);
	udm.AddArray("triangles", GetTriangles(), udm::ArrayType::Compressed);

	udm["volume"] = GetVolume();
	udm["centerOfMass"] = GetCenterOfMass();
	udm["mass"] = GetMass();

	udm["flags"]["convex"] = IsConvex();

	// Soft-body
	auto softBody = IsSoftBody();
	auto *sbInfo = GetSoftBodyInfo();
	auto *sbMesh = GetSoftBodyMesh();
	auto *sbTriangles = GetSoftBodyTriangles();
	auto *sbAnchors = GetSoftBodyAnchors();
	softBody = (softBody && sbInfo != nullptr && sbMesh != nullptr && sbTriangles != nullptr && sbAnchors != nullptr) ? true : false;
	auto meshGroupId = std::numeric_limits<uint32_t>::max();
	auto meshId = std::numeric_limits<uint32_t>::max();
	auto subMeshId = std::numeric_limits<uint32_t>::max();
	ModelSubMesh *subMesh = nullptr;
	auto foundSoftBodyMesh = false;
	if(softBody)
		softBody = mdl.FindSubMeshIndex(nullptr, nullptr, sbMesh, meshGroupId, meshId, subMeshId);

	if(softBody) {
		auto udmSoftBody = udm["softBody"];
		udmSoftBody["meshGroup"] = meshGroupId;
		udmSoftBody["mesh"] = meshId;
		udmSoftBody["subMesh"] = subMeshId;

		auto udmSettings = udmSoftBody["settings"];
		udmSettings["poseMatchingCoefficient"] = sbInfo->poseMatchingCoefficient;
		udmSettings["anchorsHardness"] = sbInfo->anchorsHardness;
		udmSettings["dragCoefficient"] = sbInfo->dragCoefficient;
		udmSettings["rigidContactsHardness"] = sbInfo->rigidContactsHardness;
		udmSettings["softContactsHardness"] = sbInfo->softContactsHardness;
		udmSettings["liftCoefficient"] = sbInfo->liftCoefficient;
		udmSettings["kineticContactsHardness"] = sbInfo->kineticContactsHardness;
		udmSettings["dynamicFrictionCoefficient"] = sbInfo->dynamicFrictionCoefficient;
		udmSettings["dampingCoefficient"] = sbInfo->dampingCoefficient;
		udmSettings["volumeConversationCoefficient"] = sbInfo->volumeConversationCoefficient;
		udmSettings["softVsRigidImpulseSplitK"] = sbInfo->softVsRigidImpulseSplitK;
		udmSettings["softVsRigidImpulseSplitR"] = sbInfo->softVsRigidImpulseSplitR;
		udmSettings["softVsRigidImpulseSplitS"] = sbInfo->softVsRigidImpulseSplitS;
		udmSettings["softVsKineticHardness"] = sbInfo->softVsKineticHardness;
		udmSettings["softVsRigidHardness"] = sbInfo->softVsRigidHardness;
		udmSettings["softVsSoftHardness"] = sbInfo->softVsSoftHardness;
		udmSettings["pressureCoefficient"] = sbInfo->pressureCoefficient;
		udmSettings["velocitiesCorrectionFactor"] = sbInfo->velocitiesCorrectionFactor;
		udmSettings["bendingConstraintsDistance"] = sbInfo->bendingConstraintsDistance;
		udmSettings["clusterCount"] = sbInfo->clusterCount;
		udmSettings["maxClusterIterations"] = sbInfo->maxClusterIterations;

		auto udmMaterialStiffnessCoefficient = udmSettings.AddArray("materialStiffnessCoefficients", sbInfo->materialStiffnessCoefficient.size());
		uint32_t idx = 0;
		for(auto &pair : sbInfo->materialStiffnessCoefficient) {
			auto udmData = udmMaterialStiffnessCoefficient[idx++];

			udmData["materialIndex"] = pair.first;
			udmData["angular"] = pair.second.angular;
			udmData["linear"] = pair.second.linear;
			udmData["volume"] = pair.second.volume;
		}

		if(sbTriangles)
			udmSettings.AddArray("triangles", *sbTriangles, udm::ArrayType::Compressed);
		if(sbAnchors) {
			static_assert(sizeof(SoftBodyAnchor) == 11);
			auto strctAnchor = ::udm::StructDescription::Define<uint16_t, uint32_t, float, uint8_t>({"vert", "bone", "influence", "flags"});
			udmSettings.AddArray("anchors", strctAnchor, *sbAnchors, udm::ArrayType::Compressed);
		}
	}
	return true;
}
bool CollisionMesh::LoadFromAssetData(Game &game, Model &mdl, const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PCOL_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}

	std::string uuid;
	if(udm["uuid"](uuid))
		m_uuid = util::uuid_string_to_bytes(uuid);

	umath::Transform pose {};
	pose.SetOrigin(GetOrigin());
	udm["bone"](m_boneID);
	udm["pose"](pose);
	std::string surfaceMaterial;
	udm["surfaceMaterial"](surfaceMaterial);

	auto *surfMat = game.GetSurfaceMaterial(surfaceMaterial);
	if(surfMat)
		m_surfaceMaterialId = surfMat->GetIndex();

	m_origin = pose.GetOrigin();

	udm["bounds"]["min"](m_min);
	udm["bounds"]["max"](m_max);

	udm["vertices"](GetVertices());
	udm["triangles"](GetTriangles());

	udm["volume"](m_volume);
	udm["centerOfMass"](m_centerOfMass);
	udm["mass"](m_mass);

	udm["flags"]["convex"](m_bConvex);

	// Soft-body
	auto udmSoftBody = udm["softBody"];
	if(udmSoftBody) {
		SetSoftBody(true);
		auto meshGroupId = std::numeric_limits<uint32_t>::max();
		auto meshId = std::numeric_limits<uint32_t>::max();
		auto subMeshId = std::numeric_limits<uint32_t>::max();
		udmSoftBody["meshGroup"](meshGroupId);
		udmSoftBody["mesh"](meshId);
		udmSoftBody["subMesh"](subMeshId);

		auto *subMesh = mdl.GetSubMesh(meshGroupId, meshId, subMeshId);
		if(subMesh)
			m_softBodyInfo->subMesh = subMesh->shared_from_this();

		auto *sbInfo = m_softBodyInfo->info.get();
		auto udmSettings = udmSoftBody["settings"];
		udmSettings["poseMatchingCoefficient"](sbInfo->poseMatchingCoefficient);
		udmSettings["anchorsHardness"](sbInfo->anchorsHardness);
		udmSettings["dragCoefficient"](sbInfo->dragCoefficient);
		udmSettings["rigidContactsHardness"](sbInfo->rigidContactsHardness);
		udmSettings["softContactsHardness"](sbInfo->softContactsHardness);
		udmSettings["liftCoefficient"](sbInfo->liftCoefficient);
		udmSettings["kineticContactsHardness"](sbInfo->kineticContactsHardness);
		udmSettings["dynamicFrictionCoefficient"](sbInfo->dynamicFrictionCoefficient);
		udmSettings["dampingCoefficient"](sbInfo->dampingCoefficient);
		udmSettings["volumeConversationCoefficient"](sbInfo->volumeConversationCoefficient);
		udmSettings["softVsRigidImpulseSplitK"](sbInfo->softVsRigidImpulseSplitK);
		udmSettings["softVsRigidImpulseSplitR"](sbInfo->softVsRigidImpulseSplitR);
		udmSettings["softVsRigidImpulseSplitS"](sbInfo->softVsRigidImpulseSplitS);
		udmSettings["softVsKineticHardness"](sbInfo->softVsKineticHardness);
		udmSettings["softVsRigidHardness"](sbInfo->softVsRigidHardness);
		udmSettings["softVsSoftHardness"](sbInfo->softVsSoftHardness);
		udmSettings["pressureCoefficient"](sbInfo->pressureCoefficient);
		udmSettings["velocitiesCorrectionFactor"](sbInfo->velocitiesCorrectionFactor);
		udmSettings["bendingConstraintsDistance"](sbInfo->bendingConstraintsDistance);
		udmSettings["clusterCount"](sbInfo->clusterCount);
		udmSettings["maxClusterIterations"](sbInfo->maxClusterIterations);

		auto udmMaterialStiffnessCoefficients = udmSettings["materialStiffnessCoefficients"];
		auto numMaterialStiffnessCoefficients = udmMaterialStiffnessCoefficients.GetSize();
		sbInfo->materialStiffnessCoefficient.reserve(numMaterialStiffnessCoefficients);
		for(auto i = decltype(numMaterialStiffnessCoefficients) {0u}; i < numMaterialStiffnessCoefficients; ++i) {
			auto udmData = udmMaterialStiffnessCoefficients[i];
			uint32_t materialIndex = std::numeric_limits<uint32_t>::max();
			udmData["materialIndex"](materialIndex);
			if(materialIndex == std::numeric_limits<uint32_t>::max())
				continue;
			PhysSoftBodyInfo::MaterialStiffnessCoefficient data {};
			udmData["angular"](data.angular);
			udmData["linear"](data.linear);
			udmData["volume"](data.volume);

			sbInfo->materialStiffnessCoefficient[materialIndex] = data;
		}

		auto *sbTriangles = GetSoftBodyTriangles();
		auto *sbAnchors = GetSoftBodyAnchors();
		udmSettings["triangles"](*sbTriangles);
		udmSettings["anchors"](*sbAnchors);
	}

	return true;
}

std::ostream &operator<<(std::ostream &out, const CollisionMesh &o)
{
	out << "CollisionMesh";
	out << "[Tris:" << o.GetTriangles().size() << "]";
	out << "[Mass:" << o.GetMass() << "]";
	out << "[Convex:" << o.IsConvex() << "]";
	out << "[CenterOfMass:" << o.GetCenterOfMass() << "]";
	out << "[Volume:" << o.GetVolume() << "]";
	out << "[SoftBody:" << o.IsSoftBody() << "]";
	out << "[Bone:" << o.GetBoneParent() << "]";
	Vector3 min, max;
	o.GetAABB(&min, &max);
	out << "[GetAABB:(" << min << "),(" << max << ")]";
	out << "[Origin:" << o.GetOrigin() << "]";
	out << "[SurfMats:" << const_cast<CollisionMesh &>(o).GetSurfaceMaterials().size() << "]";
	return out;
}
