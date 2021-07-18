/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __MODELMESH_H__
#define __MODELMESH_H__
#include "pragma/networkdefinitions.h"
#include <udm_types.hpp>
#include <vector>
#include <mathutil/glmutil.h>
#include <mathutil/vertex.hpp>
#include "pragma/model/modelupdateflags.hpp"
#include <unordered_map>
#include <mathutil/transform.hpp>

namespace umath
{
	DLLNETWORK void normalize_uv_coordinates(Vector2 &uv);
	DLLNETWORK void compute_tangent_basis(std::vector<Vertex> &verts,const std::vector<uint16_t> &triangles);
};

namespace udm {struct AssetData; using Version = uint32_t;};
class DLLNETWORK ModelSubMesh
	: public std::enable_shared_from_this<ModelSubMesh>
{
public:
	static constexpr auto PMESH_IDENTIFIER = "PMESH";
	static constexpr udm::Version PMESH_VERSION = 1;
	enum class ShareMode : uint32_t
	{
		None = 0,
		Vertices = 1,
		Alphas = 2,
		Triangles = 4,
		VertexWeights = 8,
		All = Vertices | Alphas | Triangles | VertexWeights
	};
	enum class GeometryType : uint8_t
	{
		Triangles = 0u,
		Lines,
		Points
	};
	ModelSubMesh();
	ModelSubMesh(const ModelSubMesh &other);
	static std::shared_ptr<ModelSubMesh> Load(const udm::AssetData &data,std::string &outErr);
	bool operator==(const ModelSubMesh &other) const;
	bool operator!=(const ModelSubMesh &other) const;
	bool IsEqual(const ModelSubMesh &other) const;
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
	std::vector<umath::Vertex> &GetVertices();
	std::vector<Vector2> &GetAlphas();
	std::vector<uint16_t> &GetTriangles();
	std::vector<umath::VertexWeight> &GetVertexWeights(); // Vertex weights 0-3
	std::vector<umath::VertexWeight> &GetExtendedVertexWeights(); // Vertex weights 0-7
	const std::vector<umath::Vertex> &GetVertices() const {return const_cast<ModelSubMesh*>(this)->GetVertices();}
	const std::vector<Vector2> &GetAlphas() const {return const_cast<ModelSubMesh*>(this)->GetAlphas();}
	const std::vector<uint16_t> &GetTriangles() const {return const_cast<ModelSubMesh*>(this)->GetTriangles();}
	const std::vector<umath::VertexWeight> &GetVertexWeights() const {return const_cast<ModelSubMesh*>(this)->GetVertexWeights();}
	const std::vector<umath::VertexWeight> &GetExtendedVertexWeights() const {return const_cast<ModelSubMesh*>(this)->GetExtendedVertexWeights();}
	void GetBounds(Vector3 &min,Vector3 &max) const;
	uint8_t GetAlphaCount() const;
	void SetAlphaCount(uint8_t numAlpha);
	uint32_t AddVertex(const umath::Vertex &v);
	void AddTriangle(const umath::Vertex &v1,const umath::Vertex &v2,const umath::Vertex &v3);
	void AddTriangle(uint32_t a,uint32_t b,uint32_t c);
	void AddLine(uint32_t idx0,uint32_t idx1);
	void AddPoint(uint32_t idx);
	virtual void Update(ModelUpdateFlags flags=ModelUpdateFlags::AllData);

	GeometryType GetGeometryType() const;
	void SetGeometryType(GeometryType type);

	void SetVertex(uint32_t idx,const umath::Vertex &v);
	void SetVertexPosition(uint32_t idx,const Vector3 &pos);
	void SetVertexNormal(uint32_t idx,const Vector3 &normal);
	void SetVertexUV(uint32_t idx,const Vector2 &uv);
	void SetVertexAlpha(uint32_t idx,const Vector2 &alpha);
	void SetVertexWeight(uint32_t idx,const umath::VertexWeight &weight);

	const std::vector<Vector2> *GetUVSet(const std::string &name) const;
	std::vector<Vector2> *GetUVSet(const std::string &name);
	const std::unordered_map<std::string,std::vector<Vector2>> &GetUVSets() const;
	std::unordered_map<std::string,std::vector<Vector2>> &GetUVSets();
	std::vector<Vector2> &AddUVSet(const std::string &name);

	umath::Vertex GetVertex(uint32_t idx) const;
	Vector3 GetVertexPosition(uint32_t idx) const;
	Vector3 GetVertexNormal(uint32_t idx) const;
	Vector2 GetVertexUV(uint32_t idx) const;
	Vector2 GetVertexAlpha(uint32_t idx) const;
	umath::VertexWeight GetVertexWeight(uint32_t idx) const;
	void Optimize();
	void Rotate(const Quat &rot);
	void Translate(const Vector3 &t);
	void Transform(const umath::ScaledTransform &pose);
	void Merge(const ModelSubMesh &other);
	void Scale(const Vector3 &scale);
	void ClipAgainstPlane(const Vector3 &n,double d,ModelSubMesh &clippedMeshA,ModelSubMesh &clippedMeshB,const std::vector<Mat4> *boneMatrices=nullptr,ModelSubMesh *clippedCoverMeshA=nullptr,ModelSubMesh *clippedCoverMeshB=nullptr);
	virtual std::shared_ptr<ModelSubMesh> Copy(bool fullCopy=false) const;

	void ApplyUVMapping(const Vector3 &nu,const Vector3 &nv,uint32_t w,uint32_t h,float ou,float ov,float su,float sv);
	void RemoveVertex(uint64_t idx);

	const umath::ScaledTransform &GetPose() const;
	umath::ScaledTransform &GetPose();
	void SetPose(const umath::ScaledTransform &pose);

	uint32_t GetReferenceId() const;
	void SetReferenceId(uint32_t refId);

	bool Save(udm::AssetDataArg outData,std::string &outErr);
	bool LoadFromAssetData(const udm::AssetData &data,std::string &outErr);
protected:
	void Copy(ModelSubMesh &other,bool fullCopy) const;
	std::vector<umath::VertexWeight> &GetVertexWeightSet(uint32_t idx);
	const std::vector<umath::VertexWeight> &GetVertexWeightSet(uint32_t idx) const;
	void ComputeTangentBasis();

	uint32_t m_skinTextureIndex;
	Vector3 m_center;
	std::shared_ptr<std::vector<umath::Vertex>> m_vertices;
	std::shared_ptr<std::vector<Vector2>> m_alphas;
	std::shared_ptr<std::unordered_map<std::string,std::vector<Vector2>>> m_uvSets;
	uint8_t m_numAlphas;
	std::shared_ptr<std::vector<uint16_t>> m_triangles;
	std::shared_ptr<std::vector<umath::VertexWeight>> m_vertexWeights;
	std::shared_ptr<std::vector<umath::VertexWeight>> m_extendedVertexWeights;
	Vector3 m_min;
	Vector3 m_max;
	GeometryType m_geometryType = GeometryType::Triangles;
	uint32_t m_referenceId = std::numeric_limits<uint32_t>::max();
	umath::ScaledTransform m_pose = umath::ScaledTransform{};
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
	bool IsEqual(const ModelMesh &other) const;
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

DLLNETWORK std::ostream &operator<<(std::ostream &out,const ModelSubMesh &o);
DLLNETWORK std::ostream &operator<<(std::ostream &out,const ModelMesh &o);

#endif