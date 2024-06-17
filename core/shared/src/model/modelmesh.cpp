/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/modelmesh.h"
#include "pragma/model/simplify.h"
#include <mathutil/uvec.h>
#include <pragma/math/intersection.h>
#include <udm.hpp>
#include "pragma/math/icosphere.h"
#include <sharedutils/util_ifile.hpp>
#include <sharedutils/magic_enum.hpp>

ModelMesh::ModelMesh() : std::enable_shared_from_this<ModelMesh>(), m_numVerts(0), m_numIndices(0) {}
ModelMesh::ModelMesh(const ModelMesh &other) : m_min(other.m_min), m_max(other.m_max), m_numVerts(other.m_numVerts), m_numIndices(other.m_numIndices), m_center(other.m_center), m_subMeshes(other.m_subMeshes), m_referenceId {other.m_referenceId}
{
	static_assert(sizeof(ModelMesh) == 104, "Update this function when making changes to this class!");
}
bool ModelMesh::operator==(const ModelMesh &other) const { return this == &other; }
bool ModelMesh::operator!=(const ModelMesh &other) const { return !operator==(other); }
bool ModelMesh::IsEqual(const ModelMesh &other) const
{
	static_assert(sizeof(ModelMesh) == 104, "Update this function when making changes to this class!");
	if(!(uvec::cmp(m_min, other.m_min) && uvec::cmp(m_max, other.m_max) && m_numVerts == other.m_numVerts && m_numIndices == other.m_numIndices && uvec::cmp(m_center, other.m_center) && m_referenceId == other.m_referenceId && m_subMeshes.size() == other.m_subMeshes.size()))
		return false;
	for(auto i = decltype(m_subMeshes.size()) {0u}; i < m_subMeshes.size(); ++i) {
		if(m_subMeshes[i]->IsEqual(*other.m_subMeshes[i]) == false)
			return false;
	}
	return true;
}
std::shared_ptr<ModelMesh> ModelMesh::Copy() const { return std::make_shared<ModelMesh>(*this); }
void ModelMesh::Rotate(const Quat &rot)
{
	for(auto &subMesh : m_subMeshes)
		subMesh->Rotate(rot);
	uvec::rotate(&m_min, rot);
	uvec::rotate(&m_max, rot);
	uvec::rotate(&m_center, rot);
}
void ModelMesh::Translate(const Vector3 &t)
{
	for(auto &subMesh : m_subMeshes)
		subMesh->Translate(t);
	m_min += t;
	m_max += t;
	m_center += t;
}
const Vector3 &ModelMesh::GetCenter() const { return m_center; }
void ModelMesh::SetCenter(const Vector3 &center) { m_center = center; }
uint32_t ModelMesh::GetVertexCount() const { return m_numVerts; }
uint32_t ModelMesh::GetIndexCount() const { return m_numIndices; }
uint32_t ModelMesh::GetTriangleCount() const { return m_numIndices / 3; }
uint32_t ModelMesh::GetSubMeshCount() const { return static_cast<uint32_t>(m_subMeshes.size()); }
void ModelMesh::Centralize()
{
	auto &center = GetCenter();
	for(auto &subMesh : m_subMeshes)
		subMesh->Centralize(center);
	m_min -= center;
	m_max -= center;
	m_center -= center;
}
void ModelMesh::Update(ModelUpdateFlags flags)
{
	if((flags & ModelUpdateFlags::UpdatePrimitiveCounts) != ModelUpdateFlags::None) {
		m_numVerts = 0;
		m_numIndices = 0;
	}
	if((flags & ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None) {
		if(m_subMeshes.empty()) {
			m_min = {};
			m_max = {};
		}
		else {
			m_min = Vector3(std::numeric_limits<Vector3::value_type>::max(), std::numeric_limits<Vector3::value_type>::max(), std::numeric_limits<Vector3::value_type>::max());
			m_max = Vector3(std::numeric_limits<Vector3::value_type>::lowest(), std::numeric_limits<Vector3::value_type>::lowest(), std::numeric_limits<Vector3::value_type>::lowest());
		}
		m_center = {};
	}
	uint32_t vertCount = 0;
	for(auto i = decltype(m_subMeshes.size()) {0}; i < m_subMeshes.size(); ++i) {
		auto &subMesh = m_subMeshes[i];
		if((flags & ModelUpdateFlags::UpdateChildren) != ModelUpdateFlags::None)
			subMesh->Update(flags);
		if((flags & ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None) {
			Vector3 min;
			Vector3 max;
			subMesh->GetBounds(min, max);
			uvec::min(&m_min, min);
			uvec::max(&m_max, max);

			auto &verts = subMesh->GetVertices();
			vertCount += static_cast<uint32_t>(verts.size());
			for(auto &v : verts)
				m_center += v.position;
		}

		if((flags & ModelUpdateFlags::UpdatePrimitiveCounts) != ModelUpdateFlags::None) {
			m_numVerts += subMesh->GetVertexCount();
			m_numIndices += subMesh->GetIndexCount();
		}
	}
	if((flags & ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None && vertCount > 0)
		m_center /= static_cast<float>(vertCount);
}
void ModelMesh::Merge(const ModelMesh &other)
{
	m_subMeshes.reserve(m_subMeshes.size() + other.m_subMeshes.size());
	for(auto &subMesh : other.m_subMeshes)
		m_subMeshes.push_back(subMesh);
}
uint32_t ModelMesh::GetReferenceId() const { return m_referenceId; }
void ModelMesh::SetReferenceId(uint32_t refId) { m_referenceId = refId; }
void ModelMesh::AddSubMesh(const std::shared_ptr<ModelSubMesh> &subMesh) { m_subMeshes.push_back(subMesh); }
std::vector<std::shared_ptr<ModelSubMesh>> &ModelMesh::GetSubMeshes() { return m_subMeshes; }
void ModelMesh::GetBounds(Vector3 &min, Vector3 &max) const
{
	min = m_min;
	max = m_max;
}
void ModelMesh::Scale(const Vector3 &scale)
{
	for(auto &subMesh : m_subMeshes)
		subMesh->Scale(scale);
}

//////////////////////////////////////////////////

void umath::normalize_uv_coordinates(Vector2 &uv)
{
	for(auto i = 0u; i < 2u; ++i) {
		auto &v = uv[i];
		if(v == 0.f)
			continue;
		v = fmodf(v, 1.f);
		if(v == 0.f)
			v = 1.f; // Make sure values that were 1 stay 1, otherwise there might be interpolation problems later (cannot interpolate between 0 and 0)
		else if(v < 0.f)
			v += 1.f;
	}
}

ModelSubMesh::ModelSubMesh()
    : std::enable_shared_from_this<ModelSubMesh>(), m_skinTextureIndex(0), m_numAlphas(0), m_alphas(std::make_shared<std::vector<Vector2>>()), m_indexData(std::make_shared<std::vector<uint8_t>>()), m_vertexWeights(std::make_shared<std::vector<umath::VertexWeight>>()),
      m_extendedVertexWeights(std::make_shared<std::vector<umath::VertexWeight>>()), m_vertices(std::make_shared<std::vector<umath::Vertex>>()), m_uvSets {std::make_shared<std::unordered_map<std::string, std::vector<Vector2>>>()}, m_extensions {udm::Property::Create(udm::Type::Element)},
      m_uuid {util::generate_uuid_v4()}
{
}
ModelSubMesh::ModelSubMesh(const ModelSubMesh &other)
    : m_uuid {util::generate_uuid_v4()}, m_skinTextureIndex(other.m_skinTextureIndex), m_center(other.m_center), m_vertices(other.m_vertices), m_alphas(other.m_alphas), m_numAlphas(other.m_numAlphas), m_indexData(other.m_indexData), m_vertexWeights(other.m_vertexWeights),
      m_extendedVertexWeights(other.m_extendedVertexWeights), m_min(other.m_min), m_max(other.m_max), m_pose {other.m_pose}, m_uvSets {other.m_uvSets}, m_geometryType {other.m_geometryType}, m_referenceId {other.m_referenceId}, m_indexType {other.m_indexType}, m_name {other.m_name}
{
	// Copy extension data
	std::stringstream extStream {};
	ufile::OutStreamFile extStreamFileOut {std::move(extStream)};
	other.m_extensions->Write(extStreamFileOut);

	m_extensions = udm::Property::Create(udm::Type::Element);
	ufile::InStreamFile extStreamFileIn {std::move(extStreamFileOut.MoveStream())};
	m_extensions->Read(extStreamFileIn);
	//

	static_assert(sizeof(ModelSubMesh) == 280, "Update this function when making changes to this class!");
}
std::shared_ptr<ModelSubMesh> ModelSubMesh::Load(Game &game, const udm::AssetData &data, std::string &outErr)
{
	auto mesh = game.CreateModelSubMesh();
	auto result = mesh->LoadFromAssetData(data, outErr);
	return result ? mesh : nullptr;
}
const util::Uuid &ModelSubMesh::GetUuid() const { return m_uuid; }
void ModelSubMesh::SetUuid(const util::Uuid &uuid) { m_uuid = uuid; }
const std::string &ModelSubMesh::GetName() const { return m_name; }
void ModelSubMesh::SetName(const std::string &name) { m_name = name; }
udm::PropertyWrapper ModelSubMesh::GetExtensionData() const { return *m_extensions; }
bool ModelSubMesh::operator==(const ModelSubMesh &other) const { return this == &other; }
bool ModelSubMesh::operator!=(const ModelSubMesh &other) const { return !operator==(other); }
bool ModelSubMesh::IsEqual(const ModelSubMesh &other) const
{
	static_assert(sizeof(ModelSubMesh) == 280, "Update this function when making changes to this class!");
	if(!(m_skinTextureIndex == other.m_skinTextureIndex && uvec::cmp(m_center, other.m_center) && m_numAlphas == other.m_numAlphas && uvec::cmp(m_min, other.m_min) && uvec::cmp(m_max, other.m_max) && m_geometryType == other.m_geometryType && m_referenceId == other.m_referenceId
	     && static_cast<bool>(m_vertices) == static_cast<bool>(other.m_vertices) && static_cast<bool>(m_alphas) == static_cast<bool>(other.m_alphas) && static_cast<bool>(m_uvSets) == static_cast<bool>(other.m_uvSets) && static_cast<bool>(m_indexData) == static_cast<bool>(other.m_indexData)
	     && static_cast<bool>(m_vertexWeights) == static_cast<bool>(other.m_vertexWeights) && static_cast<bool>(m_extendedVertexWeights) == static_cast<bool>(other.m_extendedVertexWeights)))
		return false;
	if(m_indexType != other.m_indexType)
		return false;
	if(uvec::cmp(m_pose.GetOrigin(), other.m_pose.GetOrigin()) == false || uquat::cmp(m_pose.GetRotation(), other.m_pose.GetRotation()) == false || uvec::cmp(m_pose.GetScale(), other.m_pose.GetScale()) == false)
		return false;
	if(m_vertices) {
		for(auto i = decltype(m_vertices->size()) {0u}; i < m_vertices->size(); ++i) {
			if(uvec::cmp((*m_vertices)[i].position, (*other.m_vertices)[i].position) == false || uvec::cmp((*m_vertices)[i].normal, (*other.m_vertices)[i].normal) == false || uvec::cmp((*m_vertices)[i].tangent, (*other.m_vertices)[i].tangent) == false
			  || uvec::cmp((*m_vertices)[i].uv, (*other.m_vertices)[i].uv) == false)
				return false;
		}
	}
	if(m_alphas) {
		for(auto i = decltype(m_alphas->size()) {0u}; i < m_alphas->size(); ++i) {
			if(uvec::cmp((*m_alphas)[i], (*other.m_alphas)[i]) == false)
				return false;
		}
	}
	if(m_uvSets) {
		if(m_uvSets->size() != other.m_uvSets->size())
			return false;
		for(auto &pair : *m_uvSets) {
			auto it = other.m_uvSets->find(pair.first);
			if(it == other.m_uvSets->end())
				return false;
			for(auto i = decltype(pair.second.size()) {0u}; i < pair.second.size(); ++i) {
				if(uvec::cmp(pair.second[i], it->second[i]) == false)
					return false;
			}
		}
	}
	if(m_uvSets && *m_uvSets != *other.m_uvSets)
		return false;
	if(m_indexData) {
		if(GetIndexCount() != other.GetIndexCount())
			return false;
		auto match = true;
		VisitIndices([&other, &match](auto *indexDataSrc, uint32_t numIndicesSrc) {
			other.VisitIndices([indexDataSrc, &match](auto *indexDataDst, uint32_t numIndicesDst) {
				for(auto i = decltype(numIndicesDst) {0u}; i < numIndicesDst; ++i) {
					if(indexDataSrc[i] != indexDataDst[i]) {
						match = false;
						break;
					}
				}
			});
		});
		if(!match)
			return false;
	}
	if(m_vertexWeights) {
		for(auto i = decltype(m_vertexWeights->size()) {0u}; i < m_vertexWeights->size(); ++i) {
			if((*m_vertexWeights)[i] != (*other.m_vertexWeights)[i])
				return false;
		}
	}
	if(m_extendedVertexWeights) {
		for(auto i = decltype(m_extendedVertexWeights->size()) {0u}; i < m_extendedVertexWeights->size(); ++i) {
			if((*m_extendedVertexWeights)[i] != (*other.m_extendedVertexWeights)[i])
				return false;
		}
	}
	return true;
}
void ModelSubMesh::Copy(ModelSubMesh &cpy, bool fullCopy) const
{
	cpy.m_name = m_name;
	if(fullCopy) {
		cpy.m_vertices = std::make_shared<std::vector<umath::Vertex>>(*cpy.m_vertices);
		cpy.m_alphas = std::make_shared<std::vector<Vector2>>(*cpy.m_alphas);
		cpy.m_indexData = std::make_shared<std::vector<uint8_t>>(*cpy.m_indexData);
		cpy.m_vertexWeights = std::make_shared<std::vector<umath::VertexWeight>>(*cpy.m_vertexWeights);
		cpy.m_extendedVertexWeights = std::make_shared<std::vector<umath::VertexWeight>>(*cpy.m_extendedVertexWeights);
		cpy.m_uvSets = std::make_shared<std::unordered_map<std::string, std::vector<Vector2>>>(*cpy.m_uvSets);

		// Copy extension data
		std::stringstream extStream {};
		ufile::OutStreamFile extStreamFileOut {std::move(extStream)};
		m_extensions->Write(extStreamFileOut);

		cpy.m_extensions = udm::Property::Create(udm::Type::Element);
		ufile::InStreamFile extStreamFileIn {std::move(extStreamFileOut.MoveStream())};
		cpy.m_extensions->Read(extStreamFileIn);
		//
	}
	static_assert(sizeof(ModelSubMesh) == 280, "Update this function when making changes to this class!");
}
std::shared_ptr<ModelSubMesh> ModelSubMesh::Copy(bool fullCopy) const
{
	auto cpy = std::make_shared<ModelSubMesh>(*this);
	if(fullCopy == false)
		return cpy;
	Copy(*cpy, fullCopy);
	return cpy;
}
uint32_t ModelSubMesh::GetReferenceId() const { return m_referenceId; }
void ModelSubMesh::SetReferenceId(uint32_t refId) { m_referenceId = refId; }
const umath::ScaledTransform &ModelSubMesh::GetPose() const { return const_cast<ModelSubMesh *>(this)->GetPose(); }
umath::ScaledTransform &ModelSubMesh::GetPose() { return m_pose; }
void ModelSubMesh::SetPose(const umath::ScaledTransform &pose) { m_pose = pose; }
void ModelSubMesh::Scale(const Vector3 &scale)
{
	m_pose.SetOrigin(m_pose.GetOrigin() * scale);
	for(auto &v : *m_vertices)
		v.position *= scale;
}
void ModelSubMesh::Merge(const ModelSubMesh &other)
{
	// TODO: Take poses into account!
	m_center = (m_center + other.m_center) / 2.f;
	uvec::to_min_max(m_min, m_max, other.m_min, other.m_max);
	std::size_t vertCount = 0;
	std::size_t newVertCount = 0;
	if(other.m_vertices != nullptr) {
		if(m_vertices == nullptr)
			m_vertices = std::make_shared<std::vector<umath::Vertex>>();
		vertCount = m_vertices->size();
		m_vertices->reserve(m_vertices->size() + other.m_vertices->size());
		for(auto &v : *other.m_vertices)
			m_vertices->push_back(v);
		newVertCount = m_vertices->size();
	}

	if(other.m_indexData != nullptr) {
		if(m_indexData == nullptr)
			m_indexData = std::make_shared<std::vector<uint8_t>>();
		if(GetIndexType() == pragma::model::IndexType::UInt16) {
			// Check if we need to increase our index size
			uint32_t maxIndex = 0;
			other.VisitIndices([&maxIndex](auto *indexData, uint32_t numIndices) {
				for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i)
					maxIndex = umath::max(maxIndex, static_cast<Index32>(indexData[i]));
			});
			if(vertCount + maxIndex >= MAX_INDEX16)
				SetIndexType(pragma::model::IndexType::UInt32);
		}
		ReserveIndices(GetIndexCount() + other.GetIndexCount());
		// TODO: Use memcpy if index type matches
		other.VisitIndices([this, vertCount](auto *indexData, uint32_t numIndices) {
			for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i)
				AddIndex(vertCount + indexData[i]);
		});
	}

	if(other.m_alphas != nullptr && ((m_alphas && !m_alphas->empty()) || !other.m_alphas->empty())) {
		if(m_alphas == nullptr)
			m_alphas = std::make_shared<std::vector<Vector2>>();
		m_alphas->reserve(newVertCount);
		m_alphas->resize(vertCount);
		for(auto &alpha : *other.m_alphas)
			m_alphas->push_back(alpha);
	}

	if(other.m_vertexWeights != nullptr && ((m_vertexWeights && !m_vertexWeights->empty()) || !other.m_vertexWeights->empty())) {
		if(m_vertexWeights == nullptr)
			m_vertexWeights = std::make_shared<std::vector<umath::VertexWeight>>();
		m_vertexWeights->reserve(newVertCount);
		m_vertexWeights->resize(vertCount);
		for(auto &vw : *other.m_vertexWeights)
			m_vertexWeights->push_back(vw);
	}

	if(other.m_extendedVertexWeights != nullptr && ((m_extendedVertexWeights && !m_extendedVertexWeights->empty()) || !other.m_extendedVertexWeights->empty())) {
		if(m_extendedVertexWeights == nullptr)
			m_extendedVertexWeights = std::make_shared<std::vector<umath::VertexWeight>>();
		m_extendedVertexWeights->reserve(newVertCount);
		m_extendedVertexWeights->resize(vertCount);
		for(auto &vw : *other.m_extendedVertexWeights)
			m_extendedVertexWeights->push_back(vw);
	}

	for(auto &pair : *other.m_uvSets) {
		auto it = m_uvSets->find(pair.first);
		if(it == m_uvSets->end()) {
			it = m_uvSets->insert(std::make_pair<std::string, std::vector<Vector2>>(std::string {pair.first}, {})).first;
			it->second.resize(vertCount);
		}
		it->second.resize(newVertCount);
		memcpy(it->second.data() + vertCount, pair.second.data(), pair.second.size() * sizeof(pair.second.front()));
	}
}
void ModelSubMesh::SetShared(const ModelSubMesh &other, ShareMode mode)
{
	if((mode & ShareMode::Vertices) != ShareMode::None)
		m_vertices = other.m_vertices;
	if((mode & ShareMode::Alphas) != ShareMode::None)
		m_alphas = other.m_alphas;
	if((mode & ShareMode::Triangles) != ShareMode::None)
		m_indexData = other.m_indexData;
	if((mode & ShareMode::VertexWeights) != ShareMode::None) {
		m_vertexWeights = other.m_vertexWeights;
		m_extendedVertexWeights = other.m_extendedVertexWeights;
	}
}
void ModelSubMesh::ClearTriangles() { m_indexData = std::make_shared<std::vector<uint8_t>>(); }
void ModelSubMesh::Centralize(const Vector3 &origin)
{
	for(auto &v : *m_vertices)
		v.position -= origin;
	m_center -= origin;
	m_min -= origin;
	m_max -= origin;
}
void ModelSubMesh::NormalizeUVCoordinates()
{
	for(auto &v : *m_vertices)
		umath::normalize_uv_coordinates(v.uv);
}
void ModelSubMesh::GenerateNormals()
{
	auto &verts = GetVertices();

	// Generate list of overlapping vertices (vertices with same position, but maybe different uvs or normals)
	std::unordered_map<uint32_t, std::vector<uint32_t>> overlappingVerts;
	for(auto vertId = decltype(verts.size()) {0}; vertId < verts.size(); ++vertId) {
		auto &v0 = verts[vertId];
		for(auto vertId2 = vertId + 1; vertId2 < verts.size(); ++vertId2) {
			auto &v1 = verts[vertId2];
			if(uvec::distance_sqr(v0.position, v1.position) >= 0.001)
				continue;
			for(auto &pair : {std::pair<uint32_t, uint32_t> {vertId, vertId2}, std::pair<uint32_t, uint32_t> {vertId2, vertId}}) {
				auto it = overlappingVerts.find(pair.first);
				if(it == overlappingVerts.end())
					it = overlappingVerts.insert(std::make_pair(pair.first, std::vector<uint32_t> {})).first;
				it->second.push_back(pair.second);
			}
		}
	}

	// For each vertex, generate list of face normals that the triangle is a part of
	std::vector<std::vector<Vector3>> faceNormalsPerVertex;
	faceNormalsPerVertex.resize(verts.size());
	VisitIndices([&](auto *indexData, uint32_t numIndices) {
		for(auto i = decltype(numIndices) {0}; i < numIndices; i += 3) {
			auto idx0 = indexData[i];
			auto idx1 = indexData[i + 1];
			auto idx2 = indexData[i + 2];
			auto faceNormal = uvec::calc_face_normal(verts[idx0].position, verts[idx1].position, verts[idx2].position);
			for(auto idx : {idx0, idx1, idx2}) {
				faceNormalsPerVertex[idx].push_back(faceNormal);
				auto it = overlappingVerts.find(idx);
				if(it == overlappingVerts.end())
					continue;
				for(auto idxOverlap : it->second)
					faceNormalsPerVertex[idxOverlap].push_back(faceNormal);
			}
		}
	});

	// Calculate average face normal for each vertex
	for(uint32_t vertIdx = 0; auto &faceNormals : faceNormalsPerVertex) {
		Vector3 n {};
		if(faceNormals.empty()) {
			++vertIdx;
			continue;
		}
		for(auto &fn : faceNormals)
			n += fn;
		n /= static_cast<float>(faceNormals.size());
		uvec::normalize(&n);
		(*m_vertices)[vertIdx].normal = n;
		++vertIdx;
	}
}
void ModelSubMesh::Rotate(const Quat &rot)
{
	for(auto &v : *m_vertices) {
		uvec::rotate(&v.position, rot);
		uvec::rotate(&v.normal, rot);
		auto t = Vector3 {v.tangent.x, v.tangent.y, v.tangent.z};
		uvec::rotate(&t, rot);
		v.tangent = Vector4 {t.x, t.y, t.z, v.tangent.w};
	}
	uvec::rotate(&m_center, rot);
	uvec::rotate(&m_min, rot);
	uvec::rotate(&m_max, rot);
}
void ModelSubMesh::Translate(const Vector3 &t)
{
	for(auto &v : *m_vertices)
		v.position += t;
	m_center += t;
	m_min += t;
	m_max += t;
}
void ModelSubMesh::Transform(const umath::ScaledTransform &pose)
{
	Scale(pose.GetScale());
	Rotate(pose.GetRotation());
	Translate(pose.GetOrigin());
}
void ModelSubMesh::SetCenter(const Vector3 &center) { m_center = center; }
const Vector3 &ModelSubMesh::GetCenter() const { return m_center; }
uint32_t ModelSubMesh::GetVertexCount() const { return static_cast<uint32_t>(m_vertices->size()); }
uint32_t ModelSubMesh::GetIndexCount() const { return m_indexData->size() / size_of_index(m_indexType); }
uint32_t ModelSubMesh::GetTriangleCount() const { return GetIndexCount() / 3; }
uint32_t ModelSubMesh::GetSkinTextureIndex() const { return m_skinTextureIndex; }
void ModelSubMesh::SetIndexCount(uint32_t numIndices) { m_indexData->resize(numIndices * size_of_index(GetIndexType())); }
void ModelSubMesh::SetTriangleCount(uint32_t numTris) { SetIndexCount(numTris * 3); }
void ModelSubMesh::SetIndices(const std::vector<Index16> &indices)
{
	m_indexData->clear();
	SetIndexType(pragma::model::IndexType::UInt16);
	SetIndexCount(indices.size());
	VisitIndices([this, &indices](auto *indexData, uint32_t numIndices) { memcpy(indexData, indices.data(), numIndices * size_of_index(pragma::model::IndexType::UInt16)); });
}
void ModelSubMesh::SetIndices(const std::vector<Index32> &indices)
{
	m_indexData->clear();
	SetIndexType(pragma::model::IndexType::UInt32);
	SetIndexCount(indices.size());
	VisitIndices([this, &indices](auto *indexData, uint32_t numIndices) { memcpy(indexData, indices.data(), numIndices * size_of_index(pragma::model::IndexType::UInt32)); });
}
void ModelSubMesh::SetSkinTextureIndex(uint32_t texture) { m_skinTextureIndex = texture; }
std::vector<umath::Vertex> &ModelSubMesh::GetVertices() { return *m_vertices; }
std::vector<Vector2> &ModelSubMesh::GetAlphas() { return *m_alphas; }
std::vector<uint8_t> &ModelSubMesh::GetIndexData() { return *m_indexData; }
std::optional<ModelSubMesh::Index32> ModelSubMesh::GetIndex(uint32_t i) const
{
	if(i >= GetIndexCount())
		return {};
	ModelSubMesh::Index32 idx;
	VisitIndices([i, &idx](auto *indexData, uint32_t numIndices) { idx = indexData[i]; });
	return idx;
}
bool ModelSubMesh::SetIndex(uint32_t i, Index32 idx)
{
	if(i >= GetIndexCount())
		return false;
	VisitIndices([i, idx](auto *indexData, uint32_t numIndices) { indexData[i] = idx; });
	return true;
}
void ModelSubMesh::GetIndices(std::vector<Index32> &outIndices) const
{
	auto offset = outIndices.size();
	outIndices.resize(outIndices.size() + GetIndexCount());
	if(GetIndexType() == pragma::model::IndexType::UInt32) {
		auto &indexData = GetIndexData();
		memcpy(outIndices.data() + offset, indexData.data(), indexData.size());
		return;
	}
	VisitIndices([offset, &outIndices](auto *indexData, uint32_t numIndices) {
		for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i)
			outIndices[offset + i] = indexData[i];
	});
}
std::vector<umath::VertexWeight> &ModelSubMesh::GetVertexWeights() { return *m_vertexWeights; }
std::vector<umath::VertexWeight> &ModelSubMesh::GetExtendedVertexWeights() { return *m_extendedVertexWeights; }
uint8_t ModelSubMesh::GetAlphaCount() const { return m_numAlphas; }
void ModelSubMesh::SetAlphaCount(uint8_t numAlpha) { m_numAlphas = numAlpha; }
uint32_t ModelSubMesh::AddVertex(const umath::Vertex &v)
{
	if(m_vertices->size() == m_vertices->capacity())
		m_vertices->reserve(static_cast<uint32_t>(m_vertices->size() * 1.5f));
	m_vertices->push_back(v);
	return static_cast<uint32_t>(m_vertices->size() - 1);
}
void ModelSubMesh::AddTriangle(const umath::Vertex &v1, const umath::Vertex &v2, const umath::Vertex &v3)
{
	if(m_vertices->size() == m_vertices->capacity())
		m_vertices->reserve(static_cast<uint32_t>(m_vertices->size() * 1.5f));
	auto numVerts = m_vertices->size();
	m_vertices->push_back(v1);
	m_vertices->push_back(v2);
	m_vertices->push_back(v3);
	AddTriangle(static_cast<uint32_t>(numVerts), static_cast<uint32_t>(numVerts + 1), static_cast<uint32_t>(numVerts + 2));
}
void ModelSubMesh::ReserveIndices(size_t num) { m_indexData->reserve(num * size_of_index(GetIndexType())); }
void ModelSubMesh::ReserveVertices(size_t num) { m_vertices->reserve(num); }
void ModelSubMesh::AddIndex(Index32 index)
{
	if(m_indexData->size() == m_indexData->capacity())
		m_indexData->reserve(static_cast<uint32_t>(m_indexData->size() * 1.5f));
	if(m_indexType == pragma::model::IndexType::UInt16 && index >= std::numeric_limits<Index16>::max())
		SetIndexType(pragma::model::IndexType::UInt32);

	auto offset = GetIndexCount();
	m_indexData->resize(m_indexData->size() + size_of_index(m_indexType));
	VisitIndices([offset, index](auto *indexData, uint32_t numIndices) { indexData[offset] = index; });
}
void ModelSubMesh::AddTriangle(uint32_t a, uint32_t b, uint32_t c)
{
	AddIndex(a);
	AddIndex(b);
	AddIndex(c);
}
void ModelSubMesh::AddLine(uint32_t idx0, uint32_t idx1)
{
	AddIndex(idx0);
	AddIndex(idx1);
}
void ModelSubMesh::AddPoint(uint32_t idx) { AddIndex(idx); }
pragma::model::IndexType ModelSubMesh::GetIndexType() const { return m_indexType; }
udm::Type ModelSubMesh::GetUdmIndexType() const
{
	switch(m_indexType) {
	case pragma::model::IndexType::UInt16:
		return udm::Type::UInt16;
	case pragma::model::IndexType::UInt32:
		return udm::Type::UInt32;
	}
	return udm::Type::Invalid;
}
void ModelSubMesh::SetIndexType(pragma::model::IndexType type)
{
	if(type == m_indexType)
		return;
	auto convertIndices = [this, type]<typename TIdxSrc, typename TIdxDst>() {
		auto numIndices = GetIndexCount();
		std::vector<uint8_t> oldData = std::move(*m_indexData);
		std::vector<uint8_t> newData;
		newData.resize(numIndices * sizeof(TIdxDst));
		auto *src = reinterpret_cast<TIdxSrc *>(oldData.data());
		auto *dst = reinterpret_cast<TIdxDst *>(newData.data());
		for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i) {
			auto idx = src[i];
			if constexpr(std::numeric_limits<TIdxSrc>::max() > std::numeric_limits<TIdxDst>::max()) {
				if(idx >= std::numeric_limits<TIdxDst>::max())
					throw std::runtime_error {"Attempted to change index type of mesh from " + std::string {magic_enum::enum_name(m_indexType)} + " to " + std::string {magic_enum::enum_name(type)} + ", but index " + std::to_string(idx) + " exceeds range of target type!"};
			}
			dst[i] = idx;
		}
		*m_indexData = std::move(newData);
	};
	switch(type) {
	case pragma::model::IndexType::UInt16:
		convertIndices.template operator()<Index32, Index16>();
		break;
	case pragma::model::IndexType::UInt32:
		convertIndices.template operator()<Index16, Index32>();
		break;
	}
	m_indexType = type;
}
ModelSubMesh::GeometryType ModelSubMesh::GetGeometryType() const { return m_geometryType; }
void ModelSubMesh::SetGeometryType(GeometryType type) { m_geometryType = type; }
void ModelSubMesh::Update(ModelUpdateFlags flags)
{
	if((flags & ModelUpdateFlags::UpdateBounds) == ModelUpdateFlags::None)
		return;
	m_min = Vector3(std::numeric_limits<Vector3::value_type>::max(), std::numeric_limits<Vector3::value_type>::max(), std::numeric_limits<Vector3::value_type>::max());
	m_max = Vector3(std::numeric_limits<Vector3::value_type>::lowest(), std::numeric_limits<Vector3::value_type>::lowest(), std::numeric_limits<Vector3::value_type>::lowest());
	m_center = {};
	if(!m_vertices->empty()) {
		for(auto i = decltype(m_vertices->size()) {0}; i < m_vertices->size(); ++i) {
			auto &v = (*m_vertices)[i];
			uvec::min(&m_min, v.position);
			uvec::max(&m_max, v.position);

			m_center += v.position;
		}
		m_center /= static_cast<float>(m_vertices->size());
	}
	else {
		m_min = Vector3(0.f, 0.f, 0.f);
		m_max = Vector3(0.f, 0.f, 0.f);
	}
}
void ModelSubMesh::SetVertex(uint32_t idx, const umath::Vertex &v)
{
	if(idx >= m_vertices->size())
		return;
	(*m_vertices)[idx] = v;
}
void ModelSubMesh::SetVertexPosition(uint32_t idx, const Vector3 &pos)
{
	if(idx >= m_vertices->size())
		return;
	(*m_vertices)[idx].position = pos;
}
void ModelSubMesh::SetVertexNormal(uint32_t idx, const Vector3 &normal)
{
	if(idx >= m_vertices->size())
		return;
	(*m_vertices)[idx].normal = normal;
}
void ModelSubMesh::SetVertexUV(uint32_t idx, const Vector2 &uv)
{
	if(idx >= m_vertices->size())
		return;
	(*m_vertices)[idx].uv = uv;
}
void ModelSubMesh::SetVertexAlpha(uint32_t idx, const Vector2 &alpha)
{
	if(idx >= m_alphas->size())
		return;
	(*m_alphas)[idx] = alpha;
}
void ModelSubMesh::ComputeTangentBasis()
{
	VisitIndices([this](auto *indexData, uint32_t numIndices) { umath::compute_tangent_basis(*m_vertices, indexData, numIndices); });

	// Obsolete: Remove this block if there are no issues with the above tangent-generator
#if 0
	auto &verts = *m_vertices;
	auto &triangles = *m_triangles;
	auto numVerts = verts.size();
	for(unsigned int i=0;i<triangles.size();i+=3)
	{
		if(triangles[i] >= numVerts || triangles[i +1] >= numVerts || triangles[i +2] >= numVerts)
		{
			Con::cwar<<"Triangle vertices ("<<triangles[i]<<","<<triangles[i +1]<<","<<triangles[i +2]<<") out of bounds for mesh "<<this<<" ("<<numVerts<<" vertices)!"<<Con::endl;
			return;
		}
		auto &v0 = verts[triangles[i]];
		auto &v1 = verts[triangles[i +1]];
		auto &v2 = verts[triangles[i +2]];

		auto deltaPos1 = v1.position -v0.position;
		auto deltaPos2 = v2.position -v0.position;

		auto deltaUV1 = v1.uv -v0.uv;
		auto deltaUV2 = v2.uv -v0.uv;
		//auto deltaUV1 = Vector2(v1.uv.x,1.f -v1.uv.y) -Vector2(v0.uv.x,1.f -v0.uv.y);
		//auto deltaUV2 = Vector2(v2.uv.x,1.f -v2.uv.y) -Vector2(v0.uv.x,1.f -v0.uv.y);
		//auto deltaUV1 = Vector2(1.f -v1.uv.x,1.f -v1.uv.y) -Vector2(1.f -v0.uv.x,1.f -v0.uv.y);
		//auto deltaUV2 = Vector2(1.f -v2.uv.x,1.f -v2.uv.y) -Vector2(1.f -v0.uv.x,1.f -v0.uv.y);

		auto d = deltaUV1.x *deltaUV2.y -deltaUV1.y *deltaUV2.x;
		auto r = (d != 0.f) ? (1.f /d) : 0.f;
		auto tangent = (deltaPos1 *deltaUV2.y -deltaPos2 *deltaUV1.y) *r;
		auto biTangent = (deltaPos2 *deltaUV1.x -deltaPos1 *deltaUV2.x) *r;
		uvec::normalize(&biTangent);

		v0.tangent = {tangent,1.f};
		v1.tangent = {tangent,1.f};
		v2.tangent = {tangent,1.f};
	}
	for(unsigned int i=0;i<triangles.size();i++)
	{
		auto &v = verts[triangles[i]];
		auto &t = v.tangent;//m_tangents[triangles[i]];

		auto tmp = Vector3{t};
		tmp = tmp -v.normal *glm::dot(v.normal,tmp);
		uvec::normalize(&tmp);
		t = {tmp,1.f};
	}
#endif
}
const std::vector<umath::VertexWeight> &ModelSubMesh::GetVertexWeightSet(uint32_t idx) const { return const_cast<ModelSubMesh *>(this)->GetVertexWeightSet(idx); }
std::vector<umath::VertexWeight> &ModelSubMesh::GetVertexWeightSet(uint32_t idx) { return (idx >= 4) ? *m_extendedVertexWeights : *m_vertexWeights; }
void ModelSubMesh::SetVertexWeight(uint32_t idx, const umath::VertexWeight &weight)
{
	auto &vertexWeights = GetVertexWeightSet(0);
	if(idx >= vertexWeights.size()) {
		vertexWeights.resize(idx + 1);
		if(vertexWeights.size() == vertexWeights.capacity())
			vertexWeights.reserve(static_cast<uint32_t>(vertexWeights.size() * 1.5f));
	}
	vertexWeights.at(idx) = weight;
}
const std::vector<Vector2> *ModelSubMesh::GetUVSet(const std::string &name) const { return const_cast<ModelSubMesh *>(this)->GetUVSet(name); }
std::vector<Vector2> *ModelSubMesh::GetUVSet(const std::string &name)
{
	auto it = m_uvSets->find(name);
	return (it != m_uvSets->end()) ? &it->second : nullptr;
}
const std::unordered_map<std::string, std::vector<Vector2>> &ModelSubMesh::GetUVSets() const { return const_cast<ModelSubMesh *>(this)->GetUVSets(); }
std::unordered_map<std::string, std::vector<Vector2>> &ModelSubMesh::GetUVSets() { return *m_uvSets; }
std::vector<Vector2> &ModelSubMesh::AddUVSet(const std::string &name)
{
	auto it = m_uvSets->insert(std::make_pair(name, std::vector<Vector2> {})).first;
	return it->second;
}
umath::Vertex ModelSubMesh::GetVertex(uint32_t idx) const
{
	if(idx >= m_vertices->size())
		return {};
	return (*m_vertices)[idx];
}
Vector3 ModelSubMesh::GetVertexPosition(uint32_t idx) const
{
	if(idx >= m_vertices->size())
		return {};
	return (*m_vertices)[idx].position;
}
Vector3 ModelSubMesh::GetVertexNormal(uint32_t idx) const
{
	if(idx >= m_vertices->size())
		return {};
	return (*m_vertices)[idx].normal;
}
Vector2 ModelSubMesh::GetVertexUV(uint32_t idx) const
{
	if(idx >= m_vertices->size())
		return {};
	return (*m_vertices)[idx].uv;
}
Vector2 ModelSubMesh::GetVertexAlpha(uint32_t idx) const
{
	if(idx >= m_alphas->size())
		return {};
	return (*m_alphas)[idx];
}
umath::VertexWeight ModelSubMesh::GetVertexWeight(uint32_t idx) const
{
	auto &vertexWeights = GetVertexWeightSet(0);
	if(idx >= vertexWeights.size())
		return {};
	return vertexWeights.at(idx);
}
void ModelSubMesh::GetBounds(Vector3 &min, Vector3 &max) const
{
	min = m_min;
	max = m_max;
}
void ModelSubMesh::Optimize(double epsilon)
{
	std::vector<umath::Vertex> newVerts;
	newVerts.reserve(m_vertices->size());

	auto bCheckAlphas = (m_alphas->size() == m_vertices->size()) ? true : false;
	// TODO: Check extended weights as well!
	auto bCheckWeights = (m_vertexWeights->size() == m_vertices->size()) ? true : false;
	std::vector<Vector2> newAlphas;
	if(bCheckAlphas == true)
		newAlphas.reserve(m_alphas->size());
	std::vector<umath::VertexWeight> newVertexWeights;
	if(bCheckWeights == true)
		newVertexWeights.reserve(m_vertexWeights->size());

	std::vector<size_t> translate;
	translate.reserve(m_vertices->size());
	for(auto i = decltype(m_vertices->size()) {0}; i < m_vertices->size(); ++i) {
		auto &v = (*m_vertices)[i];
		auto *alpha = (bCheckAlphas == true) ? &(*m_alphas)[i] : nullptr;
		auto *weight = (bCheckWeights == true) ? &(*m_vertexWeights)[i] : nullptr;
		auto vertIdx = std::numeric_limits<decltype(newVerts.size())>::max();
		for(auto j = decltype(newVerts.size()) {0}; j < newVerts.size(); ++j) {
			auto &vOther = newVerts[j];
			auto *alphaOther = (j < newAlphas.size()) ? &newAlphas[j] : nullptr;
			auto *weightOther = (j < newVertexWeights.size()) ? &newVertexWeights[j] : nullptr;
			if((bCheckAlphas && alphaOther == nullptr) || (bCheckWeights && weightOther == nullptr))
				break;
			if(v.Equal(vOther, epsilon) && (bCheckAlphas == false || (umath::abs(alpha->x - alphaOther->x) <= epsilon && umath::abs(alpha->y - alphaOther->y) <= epsilon)) && (bCheckWeights == false || *weight == *weightOther)) {
				vertIdx = j;
				break;
			}
		}
		if(vertIdx == std::numeric_limits<decltype(newVerts.size())>::max()) {
			newVerts.push_back(v);
			if(bCheckAlphas == true)
				newAlphas.push_back(*alpha);
			if(bCheckWeights == true)
				newVertexWeights.push_back(*weight);
			vertIdx = newVerts.size() - 1;
		}
		translate.push_back(vertIdx);
	}
	VisitIndices([this, &translate](auto *indexData, uint32_t numIndices) {
		for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i)
			indexData[i] = translate[indexData[i]];
	});

	*m_vertices = newVerts;
	if(bCheckAlphas == true)
		*m_alphas = newAlphas;
	if(bCheckWeights == true)
		*m_vertexWeights = newVertexWeights;
}
void ModelSubMesh::ApplyUVMapping(const Vector3 &nu, const Vector3 &nv, uint32_t w, uint32_t h, float ou, float ov, float su, float sv)
{
	auto sw = (w > 0u) ? (1.f / w) : 0.f;
	auto sh = (h > 0u) ? (1.f / h) : 0.f;
	for(auto &v : *m_vertices) {
		v.uv.x = (glm::dot(v.position, nu) * sw) / su + ou * sw;
		v.uv.y = 1.f - ((glm::dot(v.position, nv) * sh) / sv + ov * sh);
	}
}
void ModelSubMesh::RemoveVertex(uint64_t idx)
{
	if(idx < m_vertices->size())
		m_vertices->erase(m_vertices->begin() + idx);
	if(idx < m_alphas->size())
		m_alphas->erase(m_alphas->begin() + idx);
	for(auto &uvSet : *m_uvSets) {
		if(idx < uvSet.second.size())
			uvSet.second.erase(uvSet.second.begin() + idx);
	}
	if(idx < m_vertexWeights->size())
		m_vertexWeights->erase(m_vertexWeights->begin() + idx);
	if(idx < m_extendedVertexWeights->size())
		m_extendedVertexWeights->erase(m_extendedVertexWeights->begin() + idx);
}

std::shared_ptr<ModelSubMesh> ModelSubMesh::Simplify(uint32_t targetVertexCount, double aggressiveness, std::vector<uint64_t> *optOutNewVertexIndexToOriginalIndex) const
{
	Simplify::newVertexIndexToOriginalIndex.clear();

	Simplify::vertices.clear();
	auto &verts = GetVertices();
	Simplify::vertices.reserve(verts.size());
	for(auto &v : verts) {
		Simplify::Vertex sv {};
		sv.p.x = v.position.x;
		sv.p.y = v.position.y;
		sv.p.z = v.position.z;
		Simplify::vertices.push_back(sv);
	}

	Simplify::triangles.clear();
	VisitIndices([&verts](auto *indexDataSrc, uint32_t numIndicesSrc) {
		Simplify::triangles.reserve(numIndicesSrc / 3);
		for(auto i = decltype(numIndicesSrc) {0u}; i < numIndicesSrc; i += 3) {
			Simplify::triangles.push_back({});
			auto &tri = Simplify::triangles.back();

			for(uint8_t j = 0; j < 3; ++j) {
				auto idx = indexDataSrc[i + j];
				tri.v[j] = idx;

				auto &uv = verts[idx].uv;
				tri.uvs[j] = vec3f {uv.x, uv.y, 0.f};
				tri.attr |= Simplify::Attributes::TEXCOORD;
			}
		}
	});

	Simplify::simplify_mesh(targetVertexCount, aggressiveness, false);

	auto cpy = Copy(true);
	auto &newVerts = cpy->GetVertices();

	newVerts.resize(Simplify::vertices.size());
	newVerts.clear();
	for(auto &v : Simplify::vertices) {
		newVerts.push_back({});
		auto &newVert = newVerts.back();
		newVert.position = {v.p.x, v.p.y, v.p.z};
	}

	auto idxType = cpy->GetUdmIndexType();
	cpy->SetIndexCount(Simplify::triangles.size() * 3);
	cpy->VisitIndices([&cpy](auto *indexDataDst, uint32_t numIndicesDst) {
		size_t idx = 0;
		for(auto &tri : Simplify::triangles) {
			for(uint8_t i = 0; i < 3; ++i) {
				indexDataDst[idx++] = tri.v[i];
				auto &uv = tri.uvs[i];
				cpy->GetVertex(tri.v[i]).uv = {uv.x, uv.y};
			}
		}
	});

	cpy->GenerateNormals();

	auto &cpyVertWeights = cpy->GetVertexWeights();
	auto &srcVertWeights = GetVertexWeights();
	if(!srcVertWeights.empty()) {
		cpyVertWeights.resize(newVerts.size());
		for(size_t idxNew = 0; idxNew < Simplify::newVertexIndexToOriginalIndex.size(); ++idxNew) {
			auto idxOld = Simplify::newVertexIndexToOriginalIndex[idxNew];
			cpyVertWeights[idxNew] = srcVertWeights[idxOld];
		}
	}

	Simplify::vertices.clear();
	Simplify::triangles.clear();
	if(optOutNewVertexIndexToOriginalIndex) {
		*optOutNewVertexIndexToOriginalIndex = std::move(Simplify::newVertexIndexToOriginalIndex);
		Simplify::newVertexIndexToOriginalIndex.clear();
	}
	return cpy;
}

bool ModelSubMesh::Save(udm::AssetDataArg outData, std::string &outErr)
{
	outData.SetAssetType(PMESH_IDENTIFIER);
	outData.SetAssetVersion(PMESH_VERSION);

	auto udm = *outData;
	udm["uuid"] = util::uuid_to_string(m_uuid);
	udm["name"] = m_name;
	udm["referenceId"] = GetReferenceId();
	udm["pose"] = GetPose();
	udm["geometryType"] = udm::enum_to_string(GetGeometryType());

	static_assert(sizeof(umath::Vertex) == 48);
	auto strctVertex = ::udm::StructDescription::Define<Vector3, Vector2, Vector3, Vector4>({"pos", "uv", "n", "t"});
	udm.AddArray("vertices", strctVertex, GetVertices(), udm::ArrayType::Compressed);
	VisitIndices([&udm](auto *indexData, uint32_t numIndices) { udm.AddArray("indices", numIndices, indexData, udm::ArrayType::Compressed); });
	udm["skinMaterialIndex"] = m_skinTextureIndex;

	auto udmUvSets = udm["uvSets"];
	for(auto &pair : GetUVSets())
		udmUvSets.AddArray(pair.first, pair.second, udm::ArrayType::Compressed);

	auto &vertexWeights = GetVertexWeights();
	if(!vertexWeights.empty()) {
		static_assert(sizeof(umath::VertexWeight) == 32);
		auto strctVertexWeight = ::udm::StructDescription::Define<Vector4i, Vector4>({"id", "w"});
		udm.AddArray("vertexWeights", strctVertexWeight, vertexWeights, udm::ArrayType::Compressed);

		auto &extBoneWeights = GetExtendedVertexWeights();
		if(!extBoneWeights.empty())
			udm.AddArray("extendedVertexWeights", strctVertexWeight, extBoneWeights, udm::ArrayType::Compressed);
	}

	auto &alphas = GetAlphas();
	udm["alphaCount"] = GetAlphaCount();
	if(!alphas.empty())
		udm.AddArray("alphas", alphas, udm::ArrayType::Compressed);

	udm["extensions"] = m_extensions;
	return true;
}
bool ModelSubMesh::LoadFromAssetData(const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PMESH_IDENTIFIER) {
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

	udm["name"](m_name);
	udm["referenceId"](m_referenceId);
	udm["pose"](m_pose);
	udm::to_enum_value<GeometryType>(udm["geometryType"], m_geometryType);

	// Backwards compatibility
	udm["vertexData"](GetVertices());
	auto udmIndexData = udm["indexData"];
	if(udmIndexData) {
		std::vector<uint16_t> indexData;
		udmIndexData(indexData);
		SetIndices(indexData);
	}

	auto udmVertices = udm["vertices"];
	udmVertices(GetVertices());
	if(udm::is_array_type(udmVertices.GetType())) {
		// Array of vec3 is also supported
		auto &a = udmVertices.GetValue<udm::Array>();
		if(a.GetValueType() == udm::Type::Vector3) {
			std::vector<Vector3> verts;
			udmVertices(verts);
			m_vertices->resize(verts.size());
			for(uint32_t idx = 0; auto &v : verts) {
				(*m_vertices)[idx].position = v;
				++idx;
			}
		}
	}

	auto udmIndices = udm["indices"];
	auto *aIndices = udmIndices.GetValuePtr<udm::Array>();
	if(aIndices) {
		auto valueType = aIndices->GetValueType();
		SetIndexType((valueType == udm::Type::UInt16) ? pragma::model::IndexType::UInt16 : pragma::model::IndexType::UInt32);
		SetIndexCount(aIndices->GetSize());
		auto &indexData = GetIndexData();
		memcpy(indexData.data(), aIndices->GetValuePtr(0), indexData.size());
	}
	udm["skinMaterialIndex"](m_skinTextureIndex);

	auto udmUvSets = udm["uvSets"];
	auto &uvSets = GetUVSets();
	uvSets.reserve(udmUvSets.GetSize());
	for(auto udmUvSet : udmUvSets.ElIt()) {
		std::vector<Vector2> uvData {};
		udmUvSet.property(uvData);
		uvSets[std::string {udmUvSet.key}] = std::move(uvData);
	}

	udm["vertexWeights"](GetVertexWeights());
	udm["extendedVertexWeights"](GetExtendedVertexWeights());
	udm["alphaCount"](m_numAlphas);
	udm["alphas"](GetAlphas());

	auto udmExtensions = udm["extensions"];
	if(udmExtensions)
		m_extensions = udmExtensions.ClaimOwnership();
	else
		m_extensions = udm::Property::Create(udm::Type::Element);
	return true;
}

std::ostream &operator<<(std::ostream &out, const ModelMesh &o)
{
	out << "ModelMesh";
	out << "[Verts:" << o.GetVertexCount() << "]";
	out << "[Indices:" << o.GetIndexCount() << "]";
	out << "[SubMeshes:" << o.GetSubMeshCount() << "]";
	out << "[Center:" << o.GetCenter() << "]";
	return out;
}

std::ostream &operator<<(std::ostream &out, const ModelSubMesh &o)
{
	out << "ModelSubMesh";
	out << "[Verts:" << o.GetVertexCount() << "]";
	out << "[Indices:" << o.GetIndexCount() << "]";
	out << "[MatIdx:" << o.GetSkinTextureIndex() << "]";
	out << "[Center:" << o.GetCenter() << "]";
	out << "[Type:" << magic_enum::enum_name(o.GetGeometryType()) << "]";

	Vector3 min, max;
	o.GetBounds(min, max);
	out << "[Bounds:(" << min << ")(" << max << ")]";
	return out;
}

pragma::model::BoxCreateInfo::BoxCreateInfo(const Vector3 &min, const Vector3 &max) : min {min}, max {max} {}
pragma::model::SphereCreateInfo::SphereCreateInfo(const Vector3 &origin, float radius) : origin {origin}, radius {radius} {}
pragma::model::CylinderCreateInfo::CylinderCreateInfo(float radius, float length) : radius {radius}, length {length} {}

pragma::model::ConeCreateInfo::ConeCreateInfo(umath::Degree angle, float length) : length {length} { endRadius = length * umath::tan(umath::deg_to_rad(angle)); }
pragma::model::ConeCreateInfo::ConeCreateInfo(float startRadius, float length, float endRadius) : startRadius {startRadius}, length {length}, endRadius {endRadius} {}

pragma::model::EllipticConeCreateInfo::EllipticConeCreateInfo(umath::Degree angleX, umath::Degree angleY, float length) : ConeCreateInfo {angleX, length} { endRadiusY = length * umath::tan(umath::deg_to_rad(angleY)); }
pragma::model::EllipticConeCreateInfo::EllipticConeCreateInfo(float startRadiusX, float startRadiusY, float length, float endRadiusX, float endRadiusY) : ConeCreateInfo {startRadiusX, length, endRadiusX}, startRadiusY {startRadiusY}, endRadiusY {endRadiusY} {}

pragma::model::CircleCreateInfo::CircleCreateInfo(float radius, bool doubleSided) : radius {radius}, doubleSided {doubleSided} {}
pragma::model::RingCreateInfo::RingCreateInfo(float innerRadius, float outerRadius, bool doubleSided) : innerRadius {innerRadius}, outerRadius {outerRadius}, doubleSided {doubleSided} {}
void pragma::model::create_quad(ModelSubMesh &mesh, const QuadCreateInfo &createInfo)
{
	auto size = createInfo.size;
	Vector3 min {-size, 0.f, -size};
	Vector3 max {size, 0.f, size};
	std::vector<Vector3> uniqueVertices {
	  min,                           // 0
	  Vector3 {max.x, min.y, min.z}, // 1
	  Vector3 {max.x, min.y, max.z}, // 2
	  Vector3 {min.x, min.y, max.z}  // 3
	};
	std::vector<Vector3> verts {uniqueVertices.at(0), uniqueVertices.at(2), uniqueVertices.at(1), uniqueVertices.at(2), uniqueVertices.at(0), uniqueVertices.at(3)};
	std::vector<Vector3> faceNormals {uvec::UP, uvec::UP};
	std::vector<::Vector2> uvs {::Vector2 {0.f, 0.f}, ::Vector2 {1.f, 1.f}, ::Vector2 {1.f, 0.f}, ::Vector2 {1.f, 1.f}, ::Vector2 {0.f, 0.f}, ::Vector2 {0.f, 1.f}};
	for(auto i = decltype(verts.size()) {0}; i < verts.size(); i += 3) {
		auto &n = faceNormals[i / 3];
		mesh.AddVertex(umath::Vertex {verts[i], uvs[i], n});
		mesh.AddVertex(umath::Vertex {verts[i + 1], uvs[i + 1], n});
		mesh.AddVertex(umath::Vertex {verts[i + 2], uvs[i + 2], n});

		mesh.AddTriangle(static_cast<uint32_t>(i), static_cast<uint32_t>(i + 1), static_cast<uint32_t>(i + 2));
	}
	mesh.SetSkinTextureIndex(0);
	mesh.Update();
}
std::shared_ptr<ModelSubMesh> pragma::model::create_quad(Game &game, const QuadCreateInfo &createInfo)
{
	auto mesh = game.CreateModelSubMesh();
	create_quad(*mesh, createInfo);
	return mesh;
}
void pragma::model::create_box(ModelSubMesh &mesh, const BoxCreateInfo &createInfo)
{
	auto &cmin = createInfo.min;
	auto &cmax = createInfo.max;

	auto min = cmin;
	auto max = cmax;
	uvec::to_min_max(min, max);
	std::vector<Vector3> uniqueVertices {
	  min,                          // 0
	  Vector3(max.x, min.y, min.z), // 1
	  Vector3(max.x, min.y, max.z), // 2
	  Vector3(max.x, max.y, min.z), // 3
	  max,                          // 4
	  Vector3(min.x, max.y, min.z), // 5
	  Vector3(min.x, min.y, max.z), // 6
	  Vector3(min.x, max.y, max.z)  // 7
	};
	std::vector<Vector3> verts {
	  uniqueVertices[0], uniqueVertices[6], uniqueVertices[7], // 1
	  uniqueVertices[0], uniqueVertices[7], uniqueVertices[5], // 1
	  uniqueVertices[3], uniqueVertices[0], uniqueVertices[5], // 2
	  uniqueVertices[3], uniqueVertices[1], uniqueVertices[0], // 2
	  uniqueVertices[2], uniqueVertices[0], uniqueVertices[1], // 3
	  uniqueVertices[2], uniqueVertices[6], uniqueVertices[0], // 3
	  uniqueVertices[7], uniqueVertices[6], uniqueVertices[2], // 4
	  uniqueVertices[4], uniqueVertices[7], uniqueVertices[2], // 4
	  uniqueVertices[4], uniqueVertices[1], uniqueVertices[3], // 5
	  uniqueVertices[1], uniqueVertices[4], uniqueVertices[2], // 5
	  uniqueVertices[4], uniqueVertices[3], uniqueVertices[5], // 6
	  uniqueVertices[4], uniqueVertices[5], uniqueVertices[7], // 6
	};
	std::vector<Vector3> faceNormals {Vector3(-1, 0, 0), Vector3(-1, 0, 0), Vector3(0, 0, -1), Vector3(0, 0, -1), Vector3(0, -1, 0), Vector3(0, -1, 0), Vector3(0, 0, 1), Vector3(0, 0, 1), Vector3(1, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 1, 0)};
	std::vector<::Vector2> uvs {
	  ::Vector2(0, 1), ::Vector2(1, 1), ::Vector2(1, 0), // 1
	  ::Vector2(0, 1), ::Vector2(1, 0), ::Vector2(0, 0), // 1
	  ::Vector2(0, 0), ::Vector2(1, 1), ::Vector2(1, 0), // 2
	  ::Vector2(0, 0), ::Vector2(0, 1), ::Vector2(1, 1), // 2
	  ::Vector2(0, 1), ::Vector2(1, 0), ::Vector2(0, 0), // 3
	  ::Vector2(0, 1), ::Vector2(1, 1), ::Vector2(1, 0), // 3
	  ::Vector2(0, 0), ::Vector2(0, 1), ::Vector2(1, 1), // 4
	  ::Vector2(1, 0), ::Vector2(0, 0), ::Vector2(1, 1), // 4
	  ::Vector2(0, 0), ::Vector2(1, 1), ::Vector2(1, 0), // 5
	  ::Vector2(1, 1), ::Vector2(0, 0), ::Vector2(0, 1), // 5
	  ::Vector2(1, 1), ::Vector2(1, 0), ::Vector2(0, 0), // 6
	  ::Vector2(1, 1), ::Vector2(0, 0), ::Vector2(0, 1)  // 6
	};
	for(auto &uv : uvs)
		uv.y = 1.f - uv.y;
	for(auto i = decltype(verts.size()) {0}; i < verts.size(); i += 3) {
		auto &n = faceNormals[i / 3];
		mesh.AddVertex(umath::Vertex {verts[i], uvs[i], n});
		mesh.AddVertex(umath::Vertex {verts[i + 1], uvs[i + 1], n});
		mesh.AddVertex(umath::Vertex {verts[i + 2], uvs[i + 2], n});

		mesh.AddTriangle(static_cast<uint32_t>(i), static_cast<uint32_t>(i + 1), static_cast<uint32_t>(i + 2));
	}
	mesh.SetSkinTextureIndex(0);
	mesh.Update();
}
std::shared_ptr<ModelSubMesh> pragma::model::create_box(Game &game, const BoxCreateInfo &createInfo)
{
	auto mesh = game.CreateModelSubMesh();
	create_box(*mesh, createInfo);
	return mesh;
}
void pragma::model::create_sphere(ModelSubMesh &mesh, const SphereCreateInfo &createInfo)
{
	auto &origin = createInfo.origin;
	auto radius = createInfo.radius;
	auto recursionLevel = createInfo.recursionLevel;

	auto &meshVerts = mesh.GetVertices();
	std::vector<Vector3> verts;
	std::vector<uint16_t> triangles;
	IcoSphere::Create(origin, radius, verts, triangles, recursionLevel);
	mesh.SetIndices(triangles);
	meshVerts.reserve(verts.size());
	for(auto &v : verts) {
		meshVerts.push_back({});
		auto &meshVert = meshVerts.back();
		meshVert.position = v;
		auto &n = meshVert.normal = uvec::get_normal(v - origin);
		meshVert.uv = {umath::atan2(n.x, n.z) / (2.f * M_PI) + 0.5f, n.y * 0.5f + 0.5f};
	}

	mesh.SetSkinTextureIndex(0);
	mesh.Update();
}
std::shared_ptr<ModelSubMesh> pragma::model::create_sphere(Game &game, const SphereCreateInfo &createInfo)
{
	auto mesh = game.CreateModelSubMesh();
	create_sphere(*mesh, createInfo);
	return mesh;
}
void pragma::model::create_cylinder(ModelSubMesh &mesh, const CylinderCreateInfo &createInfo)
{
	auto startRadius = createInfo.radius;
	auto length = createInfo.length;
	auto segmentCount = createInfo.segmentCount;

	auto rot = uquat::create_look_rotation(uvec::FORWARD, uvec::UP);

	auto &meshVerts = mesh.GetVertices();
	std::vector<Vector3> verts;
	std::vector<uint16_t> triangles;
	umath::geometry::generate_truncated_cone_mesh({}, startRadius, {0.f, 0.f, 1.f}, length, startRadius, verts, &triangles, nullptr, segmentCount);
	mesh.SetIndices(triangles);
	meshVerts.reserve(verts.size());
	for(auto &v : verts) {
		meshVerts.push_back({});
		meshVerts.back().position = v;
		// TODO: uv coordinates, etc.
	}
	mesh.GenerateNormals();
}
std::shared_ptr<ModelSubMesh> pragma::model::create_cylinder(Game &game, const CylinderCreateInfo &createInfo)
{
	auto mesh = game.CreateModelSubMesh();
	create_cylinder(*mesh, createInfo);
	return mesh;
}
static void create_cone(ModelSubMesh &mesh, const pragma::model::ConeCreateInfo &createInfo, const std::optional<std::pair<float, float>> &yRadius)
{
	auto startRadius = createInfo.startRadius;
	auto length = createInfo.length;
	auto endRadius = createInfo.endRadius;
	auto segmentCount = createInfo.segmentCount;

	auto rot = uquat::create_look_rotation(uvec::FORWARD, uvec::UP);

	auto &meshVerts = mesh.GetVertices();
	std::vector<Vector3> verts;
	std::vector<uint16_t> triangles;
	if(yRadius.has_value())
		umath::geometry::generate_truncated_elliptic_cone_mesh({}, startRadius, yRadius->first, {0.f, 0.f, 1.f}, length, endRadius, yRadius->second, verts, &triangles, nullptr, segmentCount);
	else
		umath::geometry::generate_truncated_cone_mesh({}, startRadius, {0.f, 0.f, 1.f}, length, endRadius, verts, &triangles, nullptr, segmentCount);
	mesh.SetIndices(triangles);
	meshVerts.reserve(verts.size());
	for(auto &v : verts) {
		meshVerts.push_back({});
		meshVerts.back().position = v;
		// TODO: uv coordinates, etc.
	}
	mesh.GenerateNormals();
}
void pragma::model::create_cone(ModelSubMesh &mesh, const ConeCreateInfo &createInfo) { ::create_cone(mesh, createInfo, {}); }
std::shared_ptr<ModelSubMesh> pragma::model::create_cone(Game &game, const ConeCreateInfo &createInfo)
{
	auto mesh = game.CreateModelSubMesh();
	create_cone(*mesh, createInfo);
	return mesh;
}
void pragma::model::create_elliptic_cone(ModelSubMesh &mesh, const EllipticConeCreateInfo &createInfo) { ::create_cone(mesh, createInfo, std::pair<float, float> {createInfo.startRadiusY, createInfo.endRadiusY}); }
std::shared_ptr<ModelSubMesh> pragma::model::create_elliptic_cone(Game &game, const EllipticConeCreateInfo &createInfo)
{
	auto mesh = game.CreateModelSubMesh();
	create_elliptic_cone(*mesh, createInfo);
	return mesh;
}
void pragma::model::create_circle(ModelSubMesh &mesh, const CircleCreateInfo &createInfo)
{
	RingCreateInfo ringCreateInfo {};
	ringCreateInfo.doubleSided = createInfo.doubleSided;
	ringCreateInfo.segmentCount = createInfo.segmentCount;
	ringCreateInfo.totalAngle = createInfo.totalAngle;
	ringCreateInfo.outerRadius = createInfo.radius;
	create_ring(mesh, ringCreateInfo);
}
std::shared_ptr<ModelSubMesh> pragma::model::create_circle(Game &game, const CircleCreateInfo &createInfo)
{
	auto mesh = game.CreateModelSubMesh();
	create_circle(*mesh, createInfo);
	return mesh;
}
static void add_back_face(::ModelSubMesh &mesh)
{
	mesh.VisitIndices([&mesh](auto *indexData, uint32_t numIndices) {
		auto idx0 = indexData[numIndices - 3];
		auto idx1 = indexData[numIndices - 2];
		auto idx2 = indexData[numIndices - 1];
		mesh.AddTriangle(idx0, idx2, idx1);
	});
}
void pragma::model::create_ring(ModelSubMesh &mesh, const RingCreateInfo &createInfo)
{
	auto innerRadius = createInfo.innerRadius;
	auto segmentCount = createInfo.segmentCount;
	auto doubleSided = createInfo.doubleSided;
	auto totalAngle = createInfo.totalAngle;
	auto outerRadius = createInfo.outerRadius;

	if(innerRadius.has_value() && *innerRadius == 0.f)
		innerRadius = {};
	auto stepSize = umath::round(360.f / static_cast<float>(segmentCount));

	auto &verts = mesh.GetVertices();
	auto numVerts = segmentCount;
	if(innerRadius.has_value())
		numVerts *= 2;
	else
		++numVerts;
	verts.reserve(segmentCount + 1);

	auto numIndices = segmentCount * 3;
	if(doubleSided)
		numIndices *= 2;
	if(innerRadius.has_value())
		numIndices *= 2;
	mesh.ReserveIndices(numIndices);

	if(innerRadius.has_value() == false)
		verts.push_back({});
	auto end = (totalAngle + (innerRadius.has_value() ? stepSize : 0));
	for(uint32_t i = 0; i <= end; i += stepSize) {
		auto endLoop = false;
		if(i >= totalAngle && !innerRadius) {
			i = totalAngle;
			end = totalAngle;
			endLoop = true;
		}
		auto rad = umath::deg_to_rad(i);
		if(innerRadius.has_value()) {
			verts.push_back({});
			verts.back().position = Vector3 {umath::sin(rad), 0.f, umath::cos(rad)} * *innerRadius;
		}
		verts.push_back({});
		verts.back().position = Vector3 {umath::sin(rad), 0.f, umath::cos(rad)} * outerRadius;
		if(i == 0u) {
			if(endLoop)
				break;
			continue;
		}
		if(innerRadius.has_value() == false) {
			mesh.AddTriangle(0, verts.size() - 2, verts.size() - 1);

			if(doubleSided)
				add_back_face(mesh);

			if(endLoop)
				break;
			continue;
		}
		if(i == end)
			break; // Skip last iteration
		mesh.AddTriangle(verts.size() - 1, verts.size() - 2, verts.size());
		if(doubleSided)
			add_back_face(mesh);

		mesh.AddTriangle(verts.size() - 3, verts.size() - 2, verts.size() - 1);
		if(doubleSided)
			add_back_face(mesh);
		if(endLoop)
			break;
	}
	mesh.GenerateNormals();
}
std::shared_ptr<ModelSubMesh> pragma::model::create_ring(Game &game, const RingCreateInfo &createInfo)
{
	auto mesh = game.CreateModelSubMesh();
	create_ring(*mesh, createInfo);
	return mesh;
}
