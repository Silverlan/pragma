// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/umath.h"

#include "pragma/networkdefinitions.h"
#include "mathutil/umath_geometry.hpp"
#include <mathutil/uvec.h>

module pragma.shared;

import :model.side;

static const double EPSILON = 4.9406564584125e-2;

Side::Side() { m_triangles.push_back({}); }
void Side::Update()
{
	if(m_vertices.size() > 2) {
		m_normal = glm::cross(m_vertices[1] - m_vertices[0], m_vertices[2] - m_vertices[0]);
		uvec::normalize(&m_normal);
		m_distance = -glm::dot(m_normal, m_vertices[0]);
	}
	CalculateBounds();
}

void Side::SetConvex(bool bConvex) { m_bConvex = bConvex; }
bool Side::IsConvex() const { return m_bConvex; }

void Side::Merge(Side *other)
{
	std::vector<Vector3> verts;
	auto &vertexList = other->GetVertices();
	auto *triangles = other->GetTriangles();
	auto &uvs = other->GetUVMapping();
	auto &normals = other->GetNormals();
	for(UInt i = 0; i < triangles->size(); i++) {
		auto vertId = (*triangles)[i];
		auto &vOther = vertexList[vertId];
		auto bExists = false;
		for(UInt j = 0; j < m_vertices.size(); j++) {
			auto &vThis = m_vertices[j];
			if(fabsf(vThis.x - vOther.x) <= EPSILON && fabsf(vThis.y - vOther.y) <= EPSILON && fabsf(vThis.z - vOther.z) <= EPSILON) {
				bExists = true;
				m_triangles.front().push_back(static_cast<uint16_t>(j));
				break;
			}
		}
		if(bExists == false) {
			auto &uvOther = uvs[vertId];
			auto &normalOther = normals[vertId];
			m_vertices.push_back(vOther);
			m_uvs.push_back(uvOther);
			m_normals.push_back(normalOther);
			m_triangles.front().push_back(static_cast<uint16_t>(m_vertices.size() - 1));
		}
	}
}

bool Side::HasAlpha() const { return (m_numAlphas > 0) ? true : false; }
unsigned char Side::GetAlphaCount() const { return m_numAlphas; }

void Side::SetDisplacementAlpha(unsigned char numAlpha) { m_numAlphas = numAlpha; }

bool Side::IsPointInside(Vector3 &p, double epsilon) const
{
	if(!umath::intersection::vector_in_bounds(p, m_min, m_max, static_cast<float>(EPSILON)))
		return false;
	auto numVerts = m_vertices.size();
	if(numVerts < 3)
		return false;
	Vector3 l = m_vertices[numVerts - 1];
	Vector3 f = m_vertices[0];
	Vector3 q = glm::cross(l - p, f - p);
	for(int i = 0; i < numVerts; i++) {
		Vector3 v = m_vertices[i];
		Vector3 vNext = m_vertices[(i + 1) % numVerts];
		Vector3 w = glm::cross(vNext - v, p - v);
		if(glm::dot(q, w) < -epsilon)
			return false;
	}
	return true;
}

const Vector3 &Side::GetNormal() const { return m_normal; }
float Side::GetDistance() const { return m_distance; }

void Side::CalculateBounds()
{
	uvec::zero(&m_min);
	uvec::zero(&m_max);
	for(int i = 0; i < m_vertices.size(); i++) {
		if(i == 0) {
			m_min = m_vertices[i];
			m_max = m_vertices[i];
		}
		else {
			uvec::min(&m_min, m_vertices[i]);
			uvec::max(&m_max, m_vertices[i]);
		}
	}
}

void Side::GetBounds(Vector3 *min, Vector3 *max) const
{
	*min = m_min;
	*max = m_max;
}

unsigned int Side::GetVertexCount() const { return static_cast<unsigned int>(m_vertices.size()); }

unsigned int Side::GetTriangleVertexCount() const { return static_cast<unsigned int>(m_triangles.front().size()); }

Material *Side::GetMaterial() { return m_material; }
void Side::SetMaterial(Material *mat) { m_material = mat; }

const std::vector<Vector3> &Side::GetVertices() const { return const_cast<Side *>(this)->GetVertices(); }
std::vector<Vector3> &Side::GetVertices() { return m_vertices; }
const std::vector<uint16_t> *Side::GetTriangles(uint32_t lod) const { return const_cast<Side *>(this)->GetTriangles(); }
std::vector<uint16_t> *Side::GetTriangles(uint32_t lod) { return (lod < m_triangles.size()) ? &m_triangles.at(lod) : nullptr; }
const std::vector<Vector2> &Side::GetUVMapping() const { return const_cast<Side *>(this)->GetUVMapping(); }
std::vector<Vector2> &Side::GetUVMapping() { return m_uvs; }
const std::vector<Vector3> &Side::GetNormals() const { return const_cast<Side *>(this)->GetNormals(); }
std::vector<Vector3> &Side::GetNormals() { return m_normals; }
const std::vector<Vector2> &Side::GetAlphas() const { return const_cast<Side *>(this)->GetAlphas(); }
std::vector<Vector2> &Side::GetAlphas() { return m_alphas; }

DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Side side)
{
	Vector3 min, max;
	side.GetBounds(&min, &max);
	os << "Side[" << min << "][" << max << "]";
	return os;
}

DLLNETWORK std::ostream &operator<<(std::ostream &os, const Side side)
{
	Vector3 min, max;
	side.GetBounds(&min, &max);
	os << "Side[" << min << "][" << max << "]";
	return os;
}
