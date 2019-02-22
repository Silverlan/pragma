#ifndef __C_MESH_H__
#define __C_MESH_H__
#include "pragma/clientdefinitions.h"
#include "pragma/model/mesh.h"
#include <memory>

// Obsolete!
class DLLCLIENT CMesh
	: public Mesh
{
private:
	//std::unique_ptr<GLMesh> m_glmesh = nullptr;
public:
	CMesh();
	~CMesh();
	void Calculate();
	static Mesh *GenerateCubeMesh();
	void SetMaterial(const char *texture);
};
#endif