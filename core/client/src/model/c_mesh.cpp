#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/model/c_mesh.h"

extern ClientState *client;
CMesh::CMesh()
	//: m_glmesh(nullptr)
{}

CMesh::~CMesh()
{
	//m_glmesh = nullptr;
}

void CMesh::SetMaterial(const char *texture) {m_material = client->LoadMaterial(texture);}

void CMesh::Calculate()
{
	
}

Mesh *CMesh::GenerateCubeMesh()
{
	/*CMesh *mesh = new CMesh();
	mesh->AddTriangle(Vector3(-1.0f,-1.0f,-1.0f),Vector3(-1.0f,-1.0f, 1.0f),Vector3(-1.0f, 1.0f, 1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f,-1.0f),Vector3(-1.0f,-1.0f,-1.0f),Vector3(-1.0f, 1.0f,-1.0f));
	mesh->AddTriangle(Vector3(1.0f,-1.0f, 1.0f),Vector3(-1.0f,-1.0f,-1.0f),Vector3(1.0f,-1.0f,-1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f,-1.0f),Vector3(1.0f,-1.0f,-1.0f),Vector3(-1.0f,-1.0f,-1.0f));
	mesh->AddTriangle(Vector3(-1.0f,-1.0f,-1.0f),Vector3(-1.0f, 1.0f, 1.0f),Vector3(-1.0f, 1.0f,-1.0f));
	mesh->AddTriangle(Vector3(1.0f,-1.0f, 1.0f),Vector3(-1.0f,-1.0f, 1.0f),Vector3(-1.0f,-1.0f,-1.0f));
	mesh->AddTriangle(Vector3(-1.0f, 1.0f, 1.0f),Vector3(-1.0f,-1.0f, 1.0f),Vector3(1.0f,-1.0f, 1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, 1.0f),Vector3(1.0f,-1.0f,-1.0f),Vector3(1.0f, 1.0f,-1.0f));
	mesh->AddTriangle(Vector3(1.0f,-1.0f,-1.0f),Vector3(1.0f, 1.0f, 1.0f),Vector3(1.0f,-1.0f, 1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, 1.0f),Vector3(1.0f, 1.0f,-1.0f),Vector3(-1.0f, 1.0f,-1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, 1.0f),Vector3(-1.0f, 1.0f,-1.0f),Vector3(-1.0f, 1.0f, 1.0f));
	mesh->AddTriangle(Vector3(1.0f, 1.0f, 1.0f),Vector3(-1.0f, 1.0f, 1.0f),Vector3(1.0f,-1.0f, 1.0f));
	mesh->Calculate();
	return mesh;*/
	return nullptr;
}