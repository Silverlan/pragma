/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/model/modelmesh.h"
#include <mathutil/uvec.h>
#include <pragma/math/intersection.h>

ModelMesh::ModelMesh()
	: std::enable_shared_from_this<ModelMesh>(),m_numVerts(0),m_numTriangleVerts(0)
{}
ModelMesh::ModelMesh(const ModelMesh &other)
	: m_min(other.m_min),m_max(other.m_max),m_numVerts(other.m_numVerts),
	m_numTriangleVerts(other.m_numTriangleVerts),m_center(other.m_center),
	m_subMeshes(other.m_subMeshes)
{}
bool ModelMesh::operator==(const ModelMesh &other) const {return this == &other;}
bool ModelMesh::operator!=(const ModelMesh &other) const {return !operator==(other);}
std::shared_ptr<ModelMesh> ModelMesh::Copy() const {return std::make_shared<ModelMesh>(*this);}
void ModelMesh::Rotate(const Quat &rot)
{
	for(auto &subMesh : m_subMeshes)
		subMesh->Rotate(rot);
	uvec::rotate(&m_min,rot);
	uvec::rotate(&m_max,rot);
	uvec::rotate(&m_center,rot);
}
void ModelMesh::Translate(const Vector3 &t)
{
	for(auto &subMesh : m_subMeshes)
		subMesh->Translate(t);
	m_min += t;
	m_max += t;
	m_center += t;
}
const Vector3 &ModelMesh::GetCenter() const {return m_center;}
void ModelMesh::SetCenter(const Vector3 &center) {m_center = center;}
uint32_t ModelMesh::GetVertexCount() const {return m_numVerts;}
uint32_t ModelMesh::GetTriangleVertexCount() const {return m_numTriangleVerts;}
uint32_t ModelMesh::GetTriangleCount() const {return m_numTriangleVerts /3;}
uint32_t ModelMesh::GetSubMeshCount() const {return static_cast<uint32_t>(m_subMeshes.size());}
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
	if((flags &ModelUpdateFlags::UpdatePrimitiveCounts) != ModelUpdateFlags::None)
	{
		m_numVerts = 0;
		m_numTriangleVerts = 0;
	}
	if((flags &ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None)
	{
		if(m_subMeshes.empty())
		{
			m_min = {};
			m_max = {};
		}
		else
		{
			m_min = Vector3(std::numeric_limits<Vector3::value_type>::max(),std::numeric_limits<Vector3::value_type>::max(),std::numeric_limits<Vector3::value_type>::max());
			m_max = Vector3(std::numeric_limits<Vector3::value_type>::lowest(),std::numeric_limits<Vector3::value_type>::lowest(),std::numeric_limits<Vector3::value_type>::lowest());
		}
		m_center = {};
	}
	uint32_t vertCount = 0;
	for(auto i=decltype(m_subMeshes.size()){0};i<m_subMeshes.size();++i)
	{
		auto &subMesh = m_subMeshes[i];
		if((flags &ModelUpdateFlags::UpdateChildren) != ModelUpdateFlags::None)
			subMesh->Update(flags);
		if((flags &ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None)
		{
			Vector3 min;
			Vector3 max;
			subMesh->GetBounds(min,max);
			uvec::min(&m_min,min);
			uvec::max(&m_max,max);

			auto &verts = subMesh->GetVertices();
			vertCount += static_cast<uint32_t>(verts.size());
			for(auto &v : verts)
				m_center += v.position;
		}

		if((flags &ModelUpdateFlags::UpdatePrimitiveCounts) != ModelUpdateFlags::None)
		{
			m_numVerts += subMesh->GetVertexCount();
			m_numTriangleVerts += subMesh->GetTriangleVertexCount();
		}
	}
	if((flags &ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None && vertCount > 0)
		m_center /= static_cast<float>(vertCount);
}
void ModelMesh::Merge(const ModelMesh &other)
{
	m_subMeshes.reserve(m_subMeshes.size() +other.m_subMeshes.size());
	for(auto &subMesh : other.m_subMeshes)
		m_subMeshes.push_back(subMesh);
}
uint32_t ModelMesh::GetReferenceId() const {return m_referenceId;}
void ModelMesh::SetReferenceId(uint32_t refId) {m_referenceId = refId;}
void ModelMesh::AddSubMesh(const std::shared_ptr<ModelSubMesh> &subMesh) {m_subMeshes.push_back(subMesh);}
std::vector<std::shared_ptr<ModelSubMesh>> &ModelMesh::GetSubMeshes() {return m_subMeshes;}
void ModelMesh::GetBounds(Vector3 &min,Vector3 &max) const
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
	for(auto i=0u;i<2u;++i)
	{
		auto &v = uv[i];
		if(v == 0.f)
			continue;
		v = fmodf(v,1.f);
		if(v == 0.f)
			v = 1.f; // Make sure values that were 1 stay 1, otherwise there might be interpolation problems later (cannot interpolate between 0 and 0)
		else if(v < 0.f)
			v += 1.f;
	}
}

ModelSubMesh::ModelSubMesh()
	: std::enable_shared_from_this<ModelSubMesh>(),m_skinTextureIndex(0),m_numAlphas(0),m_alphas(std::make_shared<std::vector<Vector2>>()),
	m_triangles(std::make_shared<std::vector<uint16_t>>()),m_vertexWeights(std::make_shared<std::vector<VertexWeight>>()),
	m_extendedVertexWeights(std::make_shared<std::vector<VertexWeight>>()),m_vertices(std::make_shared<std::vector<Vertex>>()),
	m_uvSets{std::make_shared<std::unordered_map<std::string,std::vector<Vector2>>>()}
{}
ModelSubMesh::ModelSubMesh(const ModelSubMesh &other)
	: m_skinTextureIndex(other.m_skinTextureIndex),m_center(other.m_center),m_vertices(other.m_vertices),
	m_alphas(other.m_alphas),m_numAlphas(other.m_numAlphas),m_triangles(other.m_triangles),
	m_vertexWeights(other.m_vertexWeights),m_extendedVertexWeights(other.m_extendedVertexWeights),m_min(other.m_min),m_max(other.m_max),
	m_pose{other.m_pose},m_uvSets{other.m_uvSets}
{}
bool ModelSubMesh::operator==(const ModelSubMesh &other) const {return this == &other;}
bool ModelSubMesh::operator!=(const ModelSubMesh &other) const {return !operator==(other);}
std::shared_ptr<ModelSubMesh> ModelSubMesh::Copy() const {return std::make_shared<ModelSubMesh>(*this);}
uint32_t ModelSubMesh::GetReferenceId() const {return m_referenceId;}
void ModelSubMesh::SetReferenceId(uint32_t refId) {m_referenceId = refId;}
const umath::ScaledTransform &ModelSubMesh::GetPose() const {return const_cast<ModelSubMesh*>(this)->GetPose();}
umath::ScaledTransform &ModelSubMesh::GetPose() {return m_pose;}
void ModelSubMesh::SetPose(const umath::ScaledTransform &pose) {m_pose = pose;}
void ModelSubMesh::Scale(const Vector3 &scale)
{
	m_pose.SetOrigin(m_pose.GetOrigin() *scale);
	for(auto &v : *m_vertices)
		v.position *= scale;
}
void ModelSubMesh::Merge(const ModelSubMesh &other)
{
	// TODO: Take poses into account!
	m_center = (m_center +other.m_center) /2.f;
	uvec::to_min_max(m_min,m_max,other.m_min,other.m_max);
	std::size_t vertCount = 0;
	std::size_t newVertCount = 0;
	if(other.m_vertices != nullptr)
	{
		if(m_vertices == nullptr)
			m_vertices = std::make_shared<std::vector<Vertex>>();
		vertCount = m_vertices->size();
		m_vertices->reserve(m_vertices->size() +other.m_vertices->size());
		for(auto &v : *other.m_vertices)
			m_vertices->push_back(v);
		newVertCount = m_vertices->size();
	}
	
	if(other.m_triangles != nullptr)
	{
		if(m_triangles == nullptr)
			m_triangles = std::make_shared<std::vector<uint16_t>>();
		m_triangles->reserve(m_triangles->size() +other.m_triangles->size());
		for(auto &idx : *other.m_triangles)
			m_triangles->push_back(vertCount +idx);
	}

	if(other.m_alphas != nullptr)
	{
		if(m_alphas == nullptr)
			m_alphas = std::make_shared<std::vector<Vector2>>();
		m_alphas->reserve(newVertCount);
		m_alphas->resize(vertCount);
		for(auto &alpha : *other.m_alphas)
			m_alphas->push_back(alpha);
	}

	if(other.m_vertexWeights != nullptr)
	{
		if(m_vertexWeights == nullptr)
			m_vertexWeights = std::make_shared<std::vector<VertexWeight>>();
		m_vertexWeights->reserve(newVertCount);
		m_vertexWeights->resize(vertCount);
		for(auto &vw : *other.m_vertexWeights)
			m_vertexWeights->push_back(vw);
	}

	if(other.m_extendedVertexWeights != nullptr)
	{
		if(m_extendedVertexWeights == nullptr)
			m_extendedVertexWeights = std::make_shared<std::vector<VertexWeight>>();
		m_extendedVertexWeights->reserve(newVertCount);
		m_extendedVertexWeights->resize(vertCount);
		for(auto &vw : *other.m_extendedVertexWeights)
			m_extendedVertexWeights->push_back(vw);
	}

	for(auto &pair : *other.m_uvSets)
	{
		auto it = m_uvSets->find(pair.first);
		if(it == m_uvSets->end())
		{
			it = m_uvSets->insert(std::make_pair<std::string,std::vector<Vector2>>(std::string{pair.first},{})).first;
			it->second.resize(vertCount);
		}
		it->second.resize(newVertCount);
		memcpy(it->second.data() +vertCount,pair.second.data(),pair.second.size() *sizeof(pair.second.front()));
	}
}
void ModelSubMesh::SetShared(const ModelSubMesh &other,ShareMode mode)
{
	if((mode &ShareMode::Vertices) != ShareMode::None)
		m_vertices = other.m_vertices;
	if((mode &ShareMode::Alphas) != ShareMode::None)
		m_alphas = other.m_alphas;
	if((mode &ShareMode::Triangles) != ShareMode::None)
		m_triangles = other.m_triangles;
	if((mode &ShareMode::VertexWeights) != ShareMode::None)
	{
		m_vertexWeights = other.m_vertexWeights;
		m_extendedVertexWeights = other.m_extendedVertexWeights;
	}
}
void ModelSubMesh::ClearTriangles()
{
	m_triangles = std::make_shared<std::vector<uint16_t>>();
}
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
	auto &triangles = GetTriangles();
	auto &verts = GetVertices();
	std::unordered_map<decltype(triangles.size()),Vector3> processed {};
	for(auto vertId=decltype(verts.size()){0};vertId<verts.size();++vertId)
	{
		auto &v = verts[vertId];
		auto &n = v.normal = {};
		for(auto i=decltype(triangles.size()){0};i<triangles.size();i+=3)
		{
			auto idx0 = triangles[i];
			auto idx1 = triangles[i +1];
			auto idx2 = triangles[i +2];
			if(idx0 == vertId || idx1 == vertId || idx2 == vertId) // Vertex is part of this triangle
			{
				auto it = processed.find(i);
				if(it != processed.end()) // Already calculated the normal for this face
				{
					n += it->second;
					continue;
				}
				auto faceNormal = uvec::calc_face_normal(verts[idx0].position,verts[idx1].position,verts[idx2].position);
				n += faceNormal;
				processed[i] = faceNormal;
			}
		}
		uvec::normalize(&n);
	}
}
void ModelSubMesh::Rotate(const Quat &rot)
{
	for(auto &v : *m_vertices)
		uvec::rotate(&v.position,rot);
	uvec::rotate(&m_center,rot);
	uvec::rotate(&m_min,rot);
	uvec::rotate(&m_max,rot);
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
const Vector3 &ModelSubMesh::GetCenter() const {return m_center;}
uint32_t ModelSubMesh::GetVertexCount() const {return static_cast<uint32_t>(m_vertices->size());}
uint32_t ModelSubMesh::GetTriangleVertexCount() const {return static_cast<uint32_t>(m_triangles->size());}
uint32_t ModelSubMesh::GetTriangleCount() const {return static_cast<uint32_t>(m_triangles->size()) /3;}
uint32_t ModelSubMesh::GetSkinTextureIndex() const {return m_skinTextureIndex;}
void ModelSubMesh::SetSkinTextureIndex(uint32_t texture) {m_skinTextureIndex = texture;}
std::vector<Vertex> &ModelSubMesh::GetVertices() {return *m_vertices;}
std::vector<Vector2> &ModelSubMesh::GetAlphas() {return *m_alphas;}
std::vector<uint16_t> &ModelSubMesh::GetTriangles() {return *m_triangles;}
std::vector<VertexWeight> &ModelSubMesh::GetVertexWeights() {return *m_vertexWeights;}
std::vector<VertexWeight> &ModelSubMesh::GetExtendedVertexWeights() {return *m_extendedVertexWeights;}
uint8_t ModelSubMesh::GetAlphaCount() const {return m_numAlphas;}
void ModelSubMesh::SetAlphaCount(uint8_t numAlpha) {m_numAlphas = numAlpha;}
uint32_t ModelSubMesh::AddVertex(const Vertex &v)
{
	if(m_vertices->size() == m_vertices->capacity())
		m_vertices->reserve(static_cast<uint32_t>(m_vertices->size() *1.5f));
	m_vertices->push_back(v);
	return static_cast<uint32_t>(m_vertices->size() -1);
}
void ModelSubMesh::AddTriangle(const Vertex &v1,const Vertex &v2,const Vertex &v3)
{
	if(m_vertices->size() == m_vertices->capacity())
		m_vertices->reserve(static_cast<uint32_t>(m_vertices->size() *1.5f));
	auto numVerts = m_vertices->size();
	m_vertices->push_back(v1);
	m_vertices->push_back(v2);
	m_vertices->push_back(v3);
	AddTriangle(static_cast<uint32_t>(numVerts),static_cast<uint32_t>(numVerts +1),static_cast<uint32_t>(numVerts +2));
}
void ModelSubMesh::AddTriangle(uint32_t a,uint32_t b,uint32_t c)
{
	if(m_triangles->size() == m_triangles->capacity())
		m_triangles->reserve(static_cast<uint32_t>(m_triangles->size() *1.5f));
	m_triangles->push_back(static_cast<uint16_t>(a));
	m_triangles->push_back(static_cast<uint16_t>(b));
	m_triangles->push_back(static_cast<uint16_t>(c));
}
void ModelSubMesh::AddLine(uint32_t idx0,uint32_t idx1)
{
	if(m_triangles->size() == m_triangles->capacity())
		m_triangles->reserve(static_cast<uint32_t>(m_triangles->size() *1.5f));
	m_triangles->push_back(idx0);
	m_triangles->push_back(idx1);
}
void ModelSubMesh::AddPoint(uint32_t idx)
{
	if(m_triangles->size() == m_triangles->capacity())
		m_triangles->reserve(static_cast<uint32_t>(m_triangles->size() *1.5f));
	m_triangles->push_back(idx);
}
ModelSubMesh::GeometryType ModelSubMesh::GetGeometryType() const {return m_geometryType;}
void ModelSubMesh::SetGeometryType(GeometryType type) {m_geometryType = type;}
void ModelSubMesh::Update(ModelUpdateFlags flags)
{
	if((flags &ModelUpdateFlags::UpdateBounds) == ModelUpdateFlags::None)
		return;
	m_min = Vector3(std::numeric_limits<Vector3::value_type>::max(),std::numeric_limits<Vector3::value_type>::max(),std::numeric_limits<Vector3::value_type>::max());
	m_max = Vector3(std::numeric_limits<Vector3::value_type>::lowest(),std::numeric_limits<Vector3::value_type>::lowest(),std::numeric_limits<Vector3::value_type>::lowest());
	m_center = {};
	if(!m_vertices->empty())
	{
		for(auto i=decltype(m_vertices->size()){0};i<m_vertices->size();++i)
		{
			auto &v = (*m_vertices)[i];
			uvec::min(&m_min,v.position);
			uvec::max(&m_max,v.position);

			m_center += v.position;
		}
		m_center /= static_cast<float>(m_vertices->size());
	}
	else
	{
		m_min = Vector3(0.f,0.f,0.f);
		m_max = Vector3(0.f,0.f,0.f);
	}
}
void ModelSubMesh::SetVertex(uint32_t idx,const Vertex &v)
{
	if(idx >= m_vertices->size())
		return;
	(*m_vertices)[idx] = v;
}
void ModelSubMesh::SetVertexPosition(uint32_t idx,const Vector3 &pos)
{
	if(idx >= m_vertices->size())
		return;
	(*m_vertices)[idx].position = pos;
}
void ModelSubMesh::SetVertexNormal(uint32_t idx,const Vector3 &normal)
{
	if(idx >= m_vertices->size())
		return;
	(*m_vertices)[idx].normal = normal;
}
void ModelSubMesh::SetVertexUV(uint32_t idx,const Vector2 &uv)
{
	if(idx >= m_vertices->size())
		return;
	(*m_vertices)[idx].uv = uv;
}
void ModelSubMesh::SetVertexAlpha(uint32_t idx,const Vector2 &alpha)
{
	if(idx >= m_alphas->size())
		return;
	(*m_alphas)[idx] = alpha;
}
void ModelSubMesh::ComputeTangentBasis()
{
	umath::compute_tangent_basis(*m_vertices,*m_triangles);

	// Obsolete: Remove this block if there are no issues with the above tangent-generator
#if 0
	auto &verts = *m_vertices;
	auto &triangles = *m_triangles;
	auto numVerts = verts.size();
	for(unsigned int i=0;i<triangles.size();i+=3)
	{
		if(triangles[i] >= numVerts || triangles[i +1] >= numVerts || triangles[i +2] >= numVerts)
		{
			Con::cwar<<"WARNING: Triangle vertices ("<<triangles[i]<<","<<triangles[i +1]<<","<<triangles[i +2]<<") out of bounds for mesh "<<this<<" ("<<numVerts<<" vertices)!"<<Con::endl;
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
const std::vector<VertexWeight> &ModelSubMesh::GetVertexWeightSet(uint32_t idx) const
{
	return const_cast<ModelSubMesh*>(this)->GetVertexWeightSet(idx);
}
std::vector<VertexWeight> &ModelSubMesh::GetVertexWeightSet(uint32_t idx)
{
	return (idx >= 4) ? *m_extendedVertexWeights : *m_vertexWeights;
}
void ModelSubMesh::SetVertexWeight(uint32_t idx,const VertexWeight &weight)
{
	auto &vertexWeights = GetVertexWeightSet(idx);
	if(idx >= vertexWeights.size())
	{
		vertexWeights.resize(idx +1);
		if(vertexWeights.size() == vertexWeights.capacity())
			vertexWeights.reserve(static_cast<uint32_t>(vertexWeights.size() *1.5f));
	}
	vertexWeights.at(idx) = weight;
}
const std::vector<Vector2> *ModelSubMesh::GetUVSet(const std::string &name) const {return const_cast<ModelSubMesh*>(this)->GetUVSet(name);}
std::vector<Vector2> *ModelSubMesh::GetUVSet(const std::string &name)
{
	auto it = m_uvSets->find(name);
	return (it != m_uvSets->end()) ? &it->second : nullptr;
}
const std::unordered_map<std::string,std::vector<Vector2>> &ModelSubMesh::GetUVSets() const {return const_cast<ModelSubMesh*>(this)->GetUVSets();}
std::unordered_map<std::string,std::vector<Vector2>> &ModelSubMesh::GetUVSets() {return *m_uvSets;}
std::vector<Vector2> &ModelSubMesh::AddUVSet(const std::string &name)
{
	auto it = m_uvSets->insert(std::make_pair(name,std::vector<Vector2>{})).first;
	return it->second;
}
Vertex ModelSubMesh::GetVertex(uint32_t idx) const
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
VertexWeight ModelSubMesh::GetVertexWeight(uint32_t idx) const
{
	auto &vertexWeights = GetVertexWeightSet(idx);
	if(idx >= vertexWeights.size())
		return {};
	return vertexWeights.at(idx);
}
void ModelSubMesh::GetBounds(Vector3 &min,Vector3 &max) const
{
	min = m_min;
	max = m_max;
}
void ModelSubMesh::Optimize()
{
	std::vector<Vertex> newVerts;
	newVerts.reserve(m_vertices->size());
	std::vector<uint16_t> newTriangles;
	newTriangles.reserve(m_triangles->size());

	auto bCheckAlphas = (m_alphas->size() == m_vertices->size()) ? true : false;
	// TODO: Check extended weights as well!
	auto bCheckWeights = (m_vertexWeights->size() == m_vertices->size()) ? true : false;
	std::vector<Vector2> newAlphas;
	if(bCheckAlphas == true)
		newAlphas.reserve(m_alphas->size());
	std::vector<VertexWeight> newVertexWeights;
	if(bCheckWeights == true)
		newVertexWeights.reserve(m_vertexWeights->size());

	std::vector<decltype(newVerts.size())> translate;
	translate.reserve(m_vertices->size());
	for(auto i=decltype(m_vertices->size()){0};i<m_vertices->size();++i)
	{
		auto &v = (*m_vertices)[i];
		auto *alpha = (bCheckAlphas == true) ? &(*m_alphas)[i] : nullptr;
		auto *weight = (bCheckWeights == true) ? &(*m_vertexWeights)[i] : nullptr;
		auto vertIdx = std::numeric_limits<decltype(newVerts.size())>::max();
		for(auto j=decltype(newVerts.size()){0};j<newVerts.size();++j)
		{
			auto &vOther = newVerts[j];
			auto *alphaOther = (j < newAlphas.size()) ? &newAlphas[j] : nullptr;
			auto *weightOther = (j < newVertexWeights.size()) ? &newVertexWeights[j] : nullptr;
			if((bCheckAlphas && alphaOther == nullptr) || (bCheckWeights && weightOther == nullptr))
				break;
			if(v == vOther && (bCheckAlphas == false || (umath::abs(alpha->x -alphaOther->x) <= VERTEX_EPSILON && umath::abs(alpha->y -alphaOther->y) <= VERTEX_EPSILON)) && (bCheckWeights == false || *weight == *weightOther))
			{
				vertIdx = j;
				break;
			}
		}
		if(vertIdx == std::numeric_limits<decltype(newVerts.size())>::max())
		{
			newVerts.push_back(v);
			if(bCheckAlphas == true)
				newAlphas.push_back(*alpha);
			if(bCheckWeights == true)
				newVertexWeights.push_back(*weight);
			vertIdx = newVerts.size() -1;
		}
		translate.push_back(vertIdx);
	}
	for(auto i : *m_triangles)
		newTriangles.push_back(static_cast<int16_t>(translate[i]));

	*m_vertices = newVerts;
	*m_triangles = newTriangles;
	if(bCheckAlphas == true)
		*m_alphas = newAlphas;
	if(bCheckWeights == true)
		*m_vertexWeights = newVertexWeights;
}
void ModelSubMesh::ApplyUVMapping(const Vector3 &nu,const Vector3 &nv,uint32_t w,uint32_t h,float ou,float ov,float su,float sv)
{
	auto sw = (w > 0u) ? (1.f /w) : 0.f;
	auto sh = (h > 0u) ? (1.f /h) : 0.f;
	for(auto &v : *m_vertices)
	{
		v.uv.x = (glm::dot(v.position,nu) *sw) /su +ou *sw;
		v.uv.y = 1.f -((glm::dot(v.position,nv) *sh) /sv +ov *sh);
	}
}
void ModelSubMesh::RemoveVertex(uint64_t idx)
{
	if(idx < m_vertices->size())
		m_vertices->erase(m_vertices->begin() +idx);
	if(idx < m_alphas->size())
		m_alphas->erase(m_alphas->begin() +idx);
	for(auto &uvSet : *m_uvSets)
	{
		if(idx < uvSet.second.size())
			uvSet.second.erase(uvSet.second.begin() +idx);
	}
	if(idx < m_vertexWeights->size())
		m_vertexWeights->erase(m_vertexWeights->begin() +idx);
	if(idx < m_extendedVertexWeights->size())
		m_extendedVertexWeights->erase(m_extendedVertexWeights->begin() +idx);
}
