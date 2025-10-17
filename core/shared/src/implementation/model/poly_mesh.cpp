// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "memory"


module pragma.shared;

import :model.poly_mesh;

static const double EPSILON = 4.9406564584125e-2; //4.9406564584125e-4;//4.94065645841247e-324;
PolyMesh::PolyMesh() {}

Vector3 *PolyMesh::GetLocalizedCenter() { return &(*m_centerLocalized); }

void PolyMesh::AddPoly(std::shared_ptr<Poly> poly) { m_polys.push_back(poly); }

unsigned int PolyMesh::GetPolyCount() { return static_cast<unsigned int>(m_polys.size()); }

int PolyMesh::BuildPolyMesh()
{
	int numPolys = static_cast<int>(m_polys.size());
	for(int i = 0; i < numPolys - 2; i++) {
		for(int j = i + 1; j < numPolys - 1; j++) {
			for(int k = j + 1; k < numPolys; k++) {
				bool legal = true;
				auto a = m_polys[i];
				auto b = m_polys[j];
				auto c = m_polys[k];

				glm::vec3 intersect;
				auto na = a->GetNormal();
				auto nb = b->GetNormal();
				auto nc = c->GetNormal();
				bool bIntersect = umath::Plane::GetPlaneIntersection(&intersect, na, nb, nc, a->GetDistance(), b->GetDistance(), c->GetDistance());
				if(bIntersect) {
					for(int m = 0; m < numPolys; m++) {
						auto poly = m_polys[m];
						float dotProd = -glm::dot(poly->GetNormal(), intersect);
						double d = poly->GetDistance();
						if(dotProd - d > EPSILON) {
							legal = false;
							break;
						}
					}
					if(legal) {
						glm::vec3 na = a->GetNormal();
						uvec::mul(&na, -1);
						a->AddUniqueVertex(intersect, na);

						glm::vec3 nb = b->GetNormal();
						uvec::mul(&nb, -1);
						b->AddUniqueVertex(intersect, nb);

						glm::vec3 nc = c->GetNormal();
						uvec::mul(&nc, -1);
						c->AddUniqueVertex(intersect, nc);
					}
				}
			}
		}
	}
	auto bHasDisplacement = false;
	for(int i = 0; i < numPolys; i++) {
		if(m_polys[i]->IsDisplacement()) {
			bHasDisplacement = true;
			unsigned int numPolys = m_polys[i]->GetVertexCount();
			if(numPolys != 4)
				return -1;
		}
		m_polys[i]->SortVertices();
		if(m_polys[i]->IsDisplacement()) {
			auto info = m_polys[i]->GetDisplacement();
			auto &vertices = m_polys[i]->GetVertices();
			for(unsigned int i = 0; i < vertices.size(); i++) {
				auto &v = vertices[i];
				if(fabsf(v.pos.x - info->startposition.x) <= EPSILON && fabsf(v.pos.y - info->startposition.y) <= EPSILON && fabsf(v.pos.z - info->startposition.z) <= EPSILON) {
					info->startpositionId = i;
					break;
				}
			}
			if(info->startpositionId == -1)
				return -2;
		}
	}
	return 0;
}

static unsigned short POLYMESH_ERROR_LEVEL = 0;
void PolyMesh::Calculate()
{
	unsigned int numPolys = GetPolyCount();
	Vector3 pos(0, 0, 0);
	uvec::max(&m_min);
	uvec::min(&m_max);
	uvec::zero(&m_centerOfMass);
	auto totalVertCount = 0u;
	for(auto &poly : m_polys) {
		if(poly->IsValid() == false)
			continue;
		totalVertCount += poly->GetVertexCount();
	}
	m_vertices.reserve(m_vertices.size() + totalVertCount);
	for(int i = numPolys - 1; i >= 0; i--) {
		if(!m_polys[i]->IsValid()) {
			if(POLYMESH_ERROR_LEVEL > 0)
				Con::cout << "WARNING: Invalid polygon '(" << *m_polys[i] << ")' for mesh '" << *this << "': Less than 3 vertices. Removing..." << Con::endl;
			m_polys.erase(m_polys.begin() + i);
			numPolys--;
		}
		else {
			uvec::add(&pos, m_polys[i]->GetCenter());
			uvec::add(&m_centerOfMass, *m_polys[i]->GetWorldPosition());
			auto &vertices = m_polys[i]->GetVertices();
			for(int j = 0; j < vertices.size(); j++) {
				auto &v = vertices[j];
				Vector3 vThis = {};
				if(HasVertex(v, &vThis))
					uvec::match(&v.pos, vThis); // Make sure the vertex in the polygon is the same as ours
				else
					m_vertices.push_back(Vector3(v.pos));
				Vector3 min = {};
				Vector3 max = {};
				m_polys[i]->GetBounds(&min, &max);
				uvec::min(&m_min, min);
				uvec::max(&m_max, max);
			}
		}
	}
	m_center = pos;
	m_center /= numPolys;
	m_centerOfMass /= numPolys;
	m_min -= m_centerOfMass;
	m_max -= m_centerOfMass;
}

Vector3 *PolyMesh::GetWorldPosition() { return &m_centerOfMass; }

void PolyMesh::debug_print()
{
	Con::cout << "Mesh '" << *this << "':" << Con::endl;
	for(int i = 0; i < m_vertices.size(); i++)
		Con::cout << "\tVertex " << i << ": (" << m_vertices[i].x << "," << m_vertices[i].y << "," << m_vertices[i].z << ")" << Con::endl;
	Con::cout << "Polys:" << Con::endl;
	for(int i = 0; i < m_polys.size(); i++)
		m_polys[i]->debug_print();
}

void PolyMesh::CenterPolys()
{
	Vector3 center(0, 0, 0);
	int numPolys = static_cast<int>(m_polys.size());
	for(int i = 0; i < numPolys; i++) {
		Vector3 centerPoly = m_polys[i]->GetCenter();
		center += centerPoly;
	}
	center /= numPolys;
	Localize(center);
}

void PolyMesh::Localize() { Localize(GetCenter()); }

void PolyMesh::Localize(const Vector3 &center)
{
	if(m_centerLocalized.has_value() == false)
		*m_centerLocalized = Vector3(center);
	else
		uvec::add(&(*m_centerLocalized), center);
	for(int i = 0; i < m_polys.size(); i++)
		m_polys[i]->Localize(center);
	Calculate();
}

Vector3 PolyMesh::GetCenter() { return m_center; }

bool PolyMesh::HasVertex(const PolyVertex &v, Vector3 *vThis)
{
	auto &pos = v.pos;
	for(int i = 0; i < m_vertices.size(); i++) {
		auto &posThis = m_vertices[i];
		if(uvec::cmp(pos, posThis)) {
			vThis->x = posThis.x;
			vThis->y = posThis.y;
			vThis->z = posThis.z;
			return true;
		}
		else {
			Vector3 diff(posThis);
			uvec::sub(&diff, pos);
			if(diff.x <= EPSILON && diff.x >= -EPSILON && diff.y <= EPSILON && diff.y >= -EPSILON && diff.z <= EPSILON && diff.z >= -EPSILON) {
				if(vThis != NULL) {
					vThis->x = posThis.x;
					vThis->y = posThis.y;
					vThis->z = posThis.z;
				}
				return true;
			}
		}
	}
	return false;
}

const std::vector<std::shared_ptr<Poly>> &PolyMesh::GetPolys() const { return const_cast<PolyMesh *>(this)->GetPolys(); }
std::vector<std::shared_ptr<Poly>> &PolyMesh::GetPolys() { return m_polys; }

bool PolyMesh::HasDisplacements() { return m_bHasDisplacements; }
void PolyMesh::SetHasDisplacements(bool b) { m_bHasDisplacements = b; }
PolyMeshInfo &PolyMesh::GetCompiledData() { return m_compiledData; }

void PolyMesh::Validate()
{
	if(m_bValid == false || m_bHasDisplacements == false)
		return;
	auto bHasDisp = m_bHasDisplacements;
	auto bCheck = false;
	for(auto it = m_polys.begin(); it != m_polys.end(); ++it) {
		auto &poly = *it;
		if(poly->IsDisplacement()) {
			bCheck = true;
			break;
		}
	}
	if(bCheck == false)
		m_bValid = false;
}
bool PolyMesh::IsValid() { return m_bValid; }

std::shared_ptr<Model> PolyMesh::GenerateModel()
{
	/*auto mdl = std::make_shared<Model>();
	auto meshCol = std::make_shared<CollisionMesh>();
	std::vector<Vector3> &colVerts = meshCol->GetVertices();
	for(int i=0;i<m_polys.size();i++) {
		Poly *poly = m_polys[i];
		std::vector<PolyVertex*> *polyVerts = poly->GetVertices();
		std::vector<Vector3> *vertexList = new std::vector<Vector3>;
		for(int j=0;j<polyVerts->size();j++)
			vertexList->push_back((*polyVerts)[j]->pos);
		std::vector<Vector3> *verts = new std::vector<Vector3>;
		std::vector<Vector2> *uvs = new std::vector<Vector2>;
		std::vector<Vector3> *normals = new std::vector<Vector3>;
		m_polys[i]->GenerateTriangleMesh(verts,uvs,normals);
		//Material *mat = m_polys[i]->GetMaterial();
		//Side *side = new Side(vertexList,verts,uvs,normals,mat);
		//mdl->AddSide(side);

		for(int i=0;i<verts->size();i++)
			colVerts.push_back((*verts)[i]);
	}
	meshCol->CalculateBounds();
	mdl->AddCollisionMesh(meshCol);
	mdl->CalculateCollisionBounds();
	mdl->CalculateRenderBounds();
	return mdl;*/
	return nullptr; // TODO
}

void PolyMesh::GetBounds(Vector3 *min, Vector3 *max)
{
	*min = m_min;
	*max = m_max;
}
