#ifndef __GLMESH_H__
#define __GLMESH_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include <vector>

class GLMeshData;
class DLLCLIENT GLMeshDataBuffer
{
private:
	struct Item
	{
		Item(UInt32 offset,UInt32 size);
		UInt32 offset;
		UInt32 size;
		Bool is_free;
	};
public:
	friend GLMeshData;
private:
	static std::unique_ptr<GLBuffer> m_buffer; // Contains Vertex, Normal, UV Data
	static std::unique_ptr<GLVertexArray> m_vao;
	static std::unique_ptr<GLVertexArray> m_vaoVertex;
	static std::vector<Item> m_items;
	static Bool m_bInitialized;
	static UInt32 m_offset;
	static UInt32 Reserve(UInt32 size,UInt32 *index);
	static void FreeItem(UInt32 index);
	static Item *GetItem(UInt32 index);
public:
	static void Initialize();
	static void Clear();
	static GLBuffer *GetBuffer();
	static GLVertexArray *GetVertexArray();
	static GLVertexArray *GetMinimalVertexArray();
	static UInt32 GetVertexSize();
};

class DLLCLIENT GLMeshData
{
private:
	UInt32 m_numVerts;
	UInt32 m_numIndices;
	UInt32 m_itemIndex;
	Bool m_bInitialized;
public:
	GLMeshData(std::vector<Vector3> &verts,std::vector<Vector3> &normals,std::vector<Vector3> &tangents,std::vector<Vector3> &biTangents,std::vector<Vector2> &uvs,std::vector<unsigned int> &indices);
	~GLMeshData();
	UInt32 GetVertexCount();
	UInt32 GetIndexCount();
	UInt32 GetBufferOffset();
	UInt32 GetBufferSize();
	UInt32 GetDataSize();
};

class DLLCLIENT GLMesh
{
private:
	GLuint m_bufUV;
	GLuint m_bufVertex;
	GLuint m_bufNormal;
	GLuint m_bufIndex;
	GLuint m_bufAlpha;

	GLuint m_vertexArrayObject;

	unsigned int m_vertexCount;
	unsigned int m_triangleCount;
public:
	GLMesh();
	GLMesh(unsigned int vertCount,unsigned int triangleCount,GLuint vert,GLuint uv,GLuint index,GLuint normal=0,GLuint vao=0);
	~GLMesh();
	GLuint GetVertexArrayObject();
	GLuint GetVertexBuffer();
	GLuint GetUVBuffer();
	GLuint GetNormalBuffer();
	GLuint GetIndexBuffer();
	GLuint GetAlphaBuffer();
	void SetVertexArrayObject(GLuint o);
	void SetVertexBuffer(GLuint buf);
	void SetUVBuffer(GLuint buf);
	void SetNormalBuffer(GLuint buf);
	void SetAlphaBuffer(GLuint buf);
	void SetIndexBuffer(GLuint buf);
	void SetVertexCount(unsigned int vertCount);
	void SetTriangleCount(unsigned int triangleCount);
	void DeleteBuffers();
	unsigned int GetVertexCount();
	unsigned int GetTriangleCount();
};
#endif