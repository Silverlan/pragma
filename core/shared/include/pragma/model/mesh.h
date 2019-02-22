#ifndef __MESH_H__
#define __MESH_H__
#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

#include <iostream>
#include <vector>
#include <pragma/console/conout.h>
#include "material.h"

class DLLNETWORK Mesh
{
	friend Con::c_cout& operator<<(Con::c_cout&,const Mesh&);
public:
	Mesh();
	virtual ~Mesh();
protected:
	std::vector<float> m_points;
	unsigned int m_numVerts;
	Material *m_material;
	unsigned int m_bufUV;
	unsigned int m_bufVertex;
public:
	void AddTriangle(const Vector3 &a,const Vector3 &b,const Vector3 &c);
	void AddTriangle(float aa,float ab,float ac,float ba,float bb,float bc,float ca,float cb,float cc);
	unsigned int GetVertexCount();
	std::vector<float> *GetPoints();
	static Mesh *GenerateCubeMesh();
	void debug_print();
	Material *GetMaterial();
	void SetMaterial(Material *material);
	virtual void SetMaterial(const char *texture);
	unsigned int GetUVBuffer();
	unsigned int GetVertexBuffer();
};
#endif