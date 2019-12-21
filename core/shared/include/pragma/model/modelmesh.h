#ifndef __MODELMESH_H__
#define __MODELMESH_H__
#include "pragma/networkdefinitions.h"
#include <vector>
#include <mathutil/glmutil.h>
#include "pragma/model/vertex.h"
#include "pragma/model/modelupdateflags.hpp"

namespace umath
{
	DLLNETWORK void normalize_uv_coordinates(Vector2 &uv);
};

class DLLNETWORK ModelSubMesh
	: public std::enable_shared_from_this<ModelSubMesh>
{
public:
	enum class DLLNETWORK ShareMode : uint32_t
	{
		None = 0,
		Vertices = 1,
		Alphas = 2,
		Triangles = 4,
		VertexWeights = 8,
		All = Vertices | Alphas | Triangles | VertexWeights
	};
	ModelSubMesh();
	ModelSubMesh(const ModelSubMesh &other);
	bool operator==(const ModelSubMesh &other) const;
	bool operator!=(const ModelSubMesh &other) const;
	void SetShared(const ModelSubMesh &other,ShareMode mode=ShareMode::All);
	void ClearTriangles();
	virtual void Centralize(const Vector3 &origin);
	const Vector3 &GetCenter() const;
	uint32_t GetVertexCount() const;
	uint32_t GetTriangleVertexCount() const;
	uint32_t GetTriangleCount() const;
	uint32_t GetSkinTextureIndex() const;
	// Only works correctly if there are no duplicate vertices
	void GenerateNormals();
	void NormalizeUVCoordinates();
	void SetSkinTextureIndex(uint32_t texture);
	std::vector<Vertex> &GetVertices();
	std::vector<Vector2> &GetAlphas();
	std::vector<uint16_t> &GetTriangles();
	std::vector<VertexWeight> &GetVertexWeights(); // Vertex weights 0-3
	std::vector<VertexWeight> &GetExtendedVertexWeights(); // Vertex weights 0-7
	void GetBounds(Vector3 &min,Vector3 &max) const;
	uint8_t GetAlphaCount() const;
	void SetAlphaCount(uint8_t numAlpha);
	uint32_t AddVertex(const Vertex &v);
	void AddTriangle(const Vertex &v1,const Vertex &v2,const Vertex &v3);
	void AddTriangle(uint32_t a,uint32_t b,uint32_t c);
	virtual void Update(ModelUpdateFlags flags=ModelUpdateFlags::AllData);

	void SetVertex(uint32_t idx,const Vertex &v);
	void SetVertexPosition(uint32_t idx,const Vector3 &pos);
	void SetVertexNormal(uint32_t idx,const Vector3 &normal);
	void SetVertexUV(uint32_t idx,const Vector2 &uv);
	void SetVertexAlpha(uint32_t idx,const Vector2 &alpha);
	void SetVertexWeight(uint32_t idx,const VertexWeight &weight);
	Vertex GetVertex(uint32_t idx) const;
	Vector3 GetVertexPosition(uint32_t idx) const;
	Vector3 GetVertexNormal(uint32_t idx) const;
	Vector2 GetVertexUV(uint32_t idx) const;
	Vector2 GetVertexAlpha(uint32_t idx) const;
	VertexWeight GetVertexWeight(uint32_t idx) const;
	void Optimize();
	void Rotate(const Quat &rot);
	void Translate(const Vector3 &t);
	void Merge(const ModelSubMesh &other);
	void Scale(const Vector3 &scale);
	void ClipAgainstPlane(const Vector3 &n,double d,ModelSubMesh &clippedMeshA,ModelSubMesh &clippedMeshB,const std::vector<Mat4> *boneMatrices=nullptr,ModelSubMesh *clippedCoverMeshA=nullptr,ModelSubMesh *clippedCoverMeshB=nullptr);
	virtual std::shared_ptr<ModelSubMesh> Copy() const;

	void ApplyUVMapping(const Vector3 &nu,const Vector3 &nv,uint32_t w,uint32_t h,float ou,float ov,float su,float sv);

	uint32_t GetReferenceId() const;
	void SetReferenceId(uint32_t refId);
protected:
	std::vector<VertexWeight> &GetVertexWeightSet(uint32_t idx);
	const std::vector<VertexWeight> &GetVertexWeightSet(uint32_t idx) const;

	uint32_t m_skinTextureIndex;
	Vector3 m_center;
	std::shared_ptr<std::vector<Vertex>> m_vertices;
	std::shared_ptr<std::vector<Vector2>> m_alphas;
	uint8_t m_numAlphas;
	std::shared_ptr<std::vector<uint16_t>> m_triangles;
	std::shared_ptr<std::vector<VertexWeight>> m_vertexWeights;
	std::shared_ptr<std::vector<VertexWeight>> m_extendedVertexWeights;
	Vector3 m_min;
	Vector3 m_max;
	uint32_t m_referenceId = std::numeric_limits<uint32_t>::max();
	void ClipAgainstPlane(const Vector3 &n,double d,ModelSubMesh &clippedMesh,const std::vector<Mat4> *boneMatrices=nullptr,ModelSubMesh *clippedCoverMesh=nullptr);
};

REGISTER_BASIC_ARITHMETIC_OPERATORS(ModelSubMesh::ShareMode)

class DLLNETWORK ModelMesh
	: public std::enable_shared_from_this<ModelMesh>
{
public:
	ModelMesh();
	ModelMesh(const ModelMesh &other);
	ModelMesh &operator=(const ModelMesh&)=delete;
	bool operator==(const ModelMesh &other) const;
	bool operator!=(const ModelMesh &other) const;
	void Centralize();
	const Vector3 &GetCenter() const;
	void SetCenter(const Vector3 &center);
	virtual void AddSubMesh(const std::shared_ptr<ModelSubMesh> &subMesh);
	std::vector<std::shared_ptr<ModelSubMesh>> &GetSubMeshes();
	uint32_t GetVertexCount() const;
	uint32_t GetTriangleVertexCount() const;
	uint32_t GetTriangleCount() const;
	uint32_t GetSubMeshCount() const;
	virtual void Update(ModelUpdateFlags flags=ModelUpdateFlags::AllData);
	void GetBounds(Vector3 &min,Vector3 &max) const;
	void Rotate(const Quat &rot);
	void Translate(const Vector3 &t);
	void Merge(const ModelMesh &other);
	void Scale(const Vector3 &scale);
	virtual std::shared_ptr<ModelMesh> Copy() const;

	uint32_t GetReferenceId() const;
	void SetReferenceId(uint32_t refId);
protected:
	Vector3 m_min;
	Vector3 m_max;
	uint32_t m_numVerts;
	uint32_t m_numTriangleVerts;
	Vector3 m_center;
	std::vector<std::shared_ptr<ModelSubMesh>> m_subMeshes;
	uint32_t m_referenceId = std::numeric_limits<uint32_t>::max();
};

#endif