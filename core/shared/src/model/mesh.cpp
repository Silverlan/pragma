/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include <pragma/definitions.h>
#include "pragma/model/mesh.h"
#include "materialmanager.h"

extern DLLNETWORK Engine *engine;

Con::c_cout &operator<<(Con::c_cout &os, const Mesh &mesh)
{
	os << "Mesh[" << &mesh << "] [" << mesh.m_numVerts << "]";
	return os;
}

Mesh::Mesh()
{
	m_numVerts = 0;
	m_bufUV = 0;
	m_bufVertex = 0;
	m_material = NULL;
}

Mesh::~Mesh() {}

unsigned int Mesh::GetUVBuffer() { return m_bufUV; }
unsigned int Mesh::GetVertexBuffer() { return m_bufVertex; }

Material *Mesh::GetMaterial() { return m_material; }
void Mesh::SetMaterial(Material *material) { m_material = material; }
void Mesh::SetMaterial(const char *texture) { m_material = engine->GetServerNetworkState()->LoadMaterial(texture); }

unsigned int Mesh::GetVertexCount() { return m_numVerts; }

void Mesh::AddTriangle(const Vector3 &a, const Vector3 &b, const Vector3 &c)
{
	m_points.push_back(a.x);
	m_points.push_back(a.y);
	m_points.push_back(a.z);

	m_points.push_back(b.x);
	m_points.push_back(b.y);
	m_points.push_back(b.z);

	m_points.push_back(c.x);
	m_points.push_back(c.y);
	m_points.push_back(c.z);

	m_numVerts += 3;
}

void Mesh::AddTriangle(float aa, float ab, float ac, float ba, float bb, float bc, float ca, float cb, float cc)
{
	m_points.push_back(aa);
	m_points.push_back(ab);
	m_points.push_back(ac);

	m_points.push_back(ba);
	m_points.push_back(bb);
	m_points.push_back(bc);

	m_points.push_back(ca);
	m_points.push_back(cb);
	m_points.push_back(cc);
	m_numVerts += 3;
}

std::vector<float> *Mesh::GetPoints() { return &m_points; }

Mesh *Mesh::GenerateCubeMesh()
{
	Mesh *mesh = new Mesh();
	mesh->AddTriangle(Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, -1.0f, 1.0f), Vector3(-1.0f, 1.0f, 1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, -1.0f), Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 1.0f, -1.0f));
	mesh->AddTriangle(Vector3(1.0f, -1.0f, 1.0f), Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, -1.0f, -1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, -1.0f), Vector3(1.0f, -1.0f, -1.0f), Vector3(-1.0f, -1.0f, -1.0f));
	mesh->AddTriangle(Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 1.0f, 1.0f), Vector3(-1.0f, 1.0f, -1.0f));
	mesh->AddTriangle(Vector3(1.0f, -1.0f, 1.0f), Vector3(-1.0f, -1.0f, 1.0f), Vector3(-1.0f, -1.0f, -1.0f));
	mesh->AddTriangle(Vector3(-1.0f, 1.0f, 1.0f), Vector3(-1.0f, -1.0f, 1.0f), Vector3(1.0f, -1.0f, 1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, 1.0f), Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, -1.0f));
	mesh->AddTriangle(Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f), Vector3(1.0f, -1.0f, 1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, 1.0f), Vector3(1.0f, 1.0f, -1.0f), Vector3(-1.0f, 1.0f, -1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, 1.0f), Vector3(-1.0f, 1.0f, -1.0f), Vector3(-1.0f, 1.0f, 1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, 1.0f), Vector3(-1.0f, 1.0f, 1.0f), Vector3(1.0f, -1.0f, 1.0f));
	return mesh;
}

void Mesh::debug_print()
{
	unsigned int numVerts = GetVertexCount();
	Con::cout << "Mesh (" << *this << "):" << Con::endl;
	for(unsigned int i = 0; i < numVerts * 3; i += 3) {
		Con::cout << "\t(" << m_points[i] << "," << m_points[i + 1] << "," << m_points[i + 1] << ")" << Con::endl;
	}
}
