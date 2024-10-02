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
#include <optional>

namespace umath {
	DLLNETWORK void normalize_uv_coordinates(Vector2 &uv);
	template<typename TIndex>
	void compute_tangent_basis(std::vector<Vertex> &verts, const TIndex *indices, uint32_t numIndices);
};

namespace udm {
	struct AssetData;
	using Version = uint32_t;
};
namespace util {
	using Uuid = std::array<uint64_t, 2>;
};

namespace pragma {
	enum class Axis : uint8_t;
};

namespace pragma::model {
	enum class IndexType : uint8_t { UInt16 = 0u, UInt32 };
};
class Game;
class DLLNETWORK ModelSubMesh : public std::enable_shared_from_this<ModelSubMesh> {
  public:
	static constexpr auto PMESH_IDENTIFIER = "PMESH";
	static constexpr udm::Version PMESH_VERSION = 1;
	enum class ShareMode : uint32_t { None = 0, Vertices = 1, Alphas = 2, Triangles = 4, VertexWeights = 8, All = Vertices | Alphas | Triangles | VertexWeights };
	enum class GeometryType : uint8_t { Triangles = 0u, Lines, Points };
	using Index16 = uint16_t;
	using Index32 = uint32_t;
	static constexpr auto MAX_INDEX16 = std::numeric_limits<Index16>::max();
	static constexpr auto MAX_INDEX32 = std::numeric_limits<Index32>::max();
	static size_t size_of_index(pragma::model::IndexType it)
	{
		switch(it) {
		case pragma::model::IndexType::UInt16:
			return sizeof(Index16);
		case pragma::model::IndexType::UInt32:
			return sizeof(Index32);
		}
		return 0;
	}

	ModelSubMesh();
	ModelSubMesh(const ModelSubMesh &other);
	static std::shared_ptr<ModelSubMesh> Load(Game &game, const udm::AssetData &data, std::string &outErr);
	bool operator==(const ModelSubMesh &other) const;
	bool operator!=(const ModelSubMesh &other) const;
	bool IsEqual(const ModelSubMesh &other) const;
	void SetShared(const ModelSubMesh &other, ShareMode mode = ShareMode::All);
	void ClearTriangles();
	virtual void Centralize(const Vector3 &origin);
	const Vector3 &GetCenter() const;
	void SetCenter(const Vector3 &center);
	uint32_t GetVertexCount() const;
	uint32_t GetIndexCount() const;
	uint32_t GetTriangleCount() const;
	uint32_t GetSkinTextureIndex() const;
	void SetIndexCount(uint32_t numIndices);
	void SetTriangleCount(uint32_t numTris);
	void SetIndices(const std::vector<Index16> &indices);
	void SetIndices(const std::vector<Index32> &indices);
	// Only works correctly if there are no duplicate vertices
	void GenerateNormals();
	void NormalizeUVCoordinates();
	void SetSkinTextureIndex(uint32_t texture);
	std::vector<umath::Vertex> &GetVertices();
	std::vector<Vector2> &GetAlphas();
	std::vector<uint8_t> &GetIndexData();
	void GetIndices(std::vector<Index32> &outIndices) const;
	std::optional<Index32> GetIndex(uint32_t i) const;
	bool SetIndex(uint32_t i, Index32 idx);
	std::vector<umath::VertexWeight> &GetVertexWeights();         // Vertex weights 0-3
	std::vector<umath::VertexWeight> &GetExtendedVertexWeights(); // Vertex weights 0-7
	const std::vector<umath::Vertex> &GetVertices() const { return const_cast<ModelSubMesh *>(this)->GetVertices(); }
	const std::vector<Vector2> &GetAlphas() const { return const_cast<ModelSubMesh *>(this)->GetAlphas(); }
	const std::vector<uint8_t> &GetIndexData() const { return const_cast<ModelSubMesh *>(this)->GetIndexData(); }
	const std::vector<umath::VertexWeight> &GetVertexWeights() const { return const_cast<ModelSubMesh *>(this)->GetVertexWeights(); }
	const std::vector<umath::VertexWeight> &GetExtendedVertexWeights() const { return const_cast<ModelSubMesh *>(this)->GetExtendedVertexWeights(); }
	void GetBounds(Vector3 &min, Vector3 &max) const;
	uint8_t GetAlphaCount() const;
	void SetAlphaCount(uint8_t numAlpha);
	uint32_t AddVertex(const umath::Vertex &v);
	void AddTriangle(const umath::Vertex &v1, const umath::Vertex &v2, const umath::Vertex &v3);
	void AddTriangle(uint32_t a, uint32_t b, uint32_t c);
	void AddIndex(Index32 index);
	void AddLine(uint32_t idx0, uint32_t idx1);
	void AddPoint(uint32_t idx);
	void ReserveIndices(size_t num);
	void ReserveVertices(size_t num);
	void Validate();
	virtual void Update(ModelUpdateFlags flags = ModelUpdateFlags::AllData);

	GeometryType GetGeometryType() const;
	void SetGeometryType(GeometryType type);

	pragma::model::IndexType GetIndexType() const;
	void SetIndexType(pragma::model::IndexType type);
	udm::Type GetUdmIndexType() const;
	void VisitIndices(auto vs)
	{
		auto &indexData = GetIndexData();
		switch(m_indexType) {
		case pragma::model::IndexType::UInt16:
			vs(reinterpret_cast<Index16 *>(indexData.data()), GetIndexCount());
			break;
		case pragma::model::IndexType::UInt32:
			vs(reinterpret_cast<Index32 *>(indexData.data()), GetIndexCount());
			break;
		}
	}
	void VisitIndices(auto vs) const
	{
		auto &indexData = GetIndexData();
		switch(m_indexType) {
		case pragma::model::IndexType::UInt16:
			vs(reinterpret_cast<const Index16 *>(indexData.data()), GetIndexCount());
			break;
		case pragma::model::IndexType::UInt32:
			vs(reinterpret_cast<const Index32 *>(indexData.data()), GetIndexCount());
			break;
		}
	}

	void SetVertex(uint32_t idx, const umath::Vertex &v);
	void SetVertexPosition(uint32_t idx, const Vector3 &pos);
	void SetVertexNormal(uint32_t idx, const Vector3 &normal);
	void SetVertexUV(uint32_t idx, const Vector2 &uv);
	void SetVertexAlpha(uint32_t idx, const Vector2 &alpha);
	void SetVertexWeight(uint32_t idx, const umath::VertexWeight &weight);

	const std::vector<Vector2> *GetUVSet(const std::string &name) const;
	std::vector<Vector2> *GetUVSet(const std::string &name);
	const std::unordered_map<std::string, std::vector<Vector2>> &GetUVSets() const;
	std::unordered_map<std::string, std::vector<Vector2>> &GetUVSets();
	std::vector<Vector2> &AddUVSet(const std::string &name);

	umath::Vertex GetVertex(uint32_t idx) const;
	Vector3 GetVertexPosition(uint32_t idx) const;
	Vector3 GetVertexNormal(uint32_t idx) const;
	Vector2 GetVertexUV(uint32_t idx) const;
	Vector2 GetVertexAlpha(uint32_t idx) const;
	umath::VertexWeight GetVertexWeight(uint32_t idx) const;
	void Optimize(double epsilon = umath::VERTEX_EPSILON);
	void Rotate(const Quat &rot);
	void Translate(const Vector3 &t);
	void Transform(const umath::ScaledTransform &pose);
	void Merge(const ModelSubMesh &other);
	void Scale(const Vector3 &scale);
	void Mirror(pragma::Axis axis);
	void ClipAgainstPlane(const Vector3 &n, double d, ModelSubMesh &clippedMeshA, ModelSubMesh &clippedMeshB, const std::vector<Mat4> *boneMatrices = nullptr, ModelSubMesh *clippedCoverMeshA = nullptr, ModelSubMesh *clippedCoverMeshB = nullptr);
	virtual std::shared_ptr<ModelSubMesh> Copy(bool fullCopy = false) const;
	std::shared_ptr<ModelSubMesh> Simplify(uint32_t targetVertexCount, double aggressiveness = 5.0, std::vector<uint64_t> *optOutNewVertexIndexToOriginalIndex = nullptr) const;

	void ApplyUVMapping(const Vector3 &nu, const Vector3 &nv, uint32_t w, uint32_t h, float ou, float ov, float su, float sv);
	void RemoveVertex(uint64_t idx);

	const umath::ScaledTransform &GetPose() const;
	umath::ScaledTransform &GetPose();
	void SetPose(const umath::ScaledTransform &pose);

	udm::PropertyWrapper GetExtensionData() const;

	uint32_t GetReferenceId() const;
	void SetReferenceId(uint32_t refId);

	const util::Uuid &GetUuid() const;
	void SetUuid(const util::Uuid &uuid);

	const std::string &GetName() const;
	void SetName(const std::string &name);

	bool Save(udm::AssetDataArg outData, std::string &outErr);
	bool LoadFromAssetData(const udm::AssetData &data, std::string &outErr);
  protected:
	void Copy(ModelSubMesh &other, bool fullCopy) const;
	std::vector<umath::VertexWeight> &GetVertexWeightSet(uint32_t idx);
	const std::vector<umath::VertexWeight> &GetVertexWeightSet(uint32_t idx) const;
	void ComputeTangentBasis();

	uint32_t m_skinTextureIndex;
	Vector3 m_center;
	std::shared_ptr<std::vector<umath::Vertex>> m_vertices;
	std::shared_ptr<std::vector<Vector2>> m_alphas;
	std::shared_ptr<std::unordered_map<std::string, std::vector<Vector2>>> m_uvSets;
	uint8_t m_numAlphas;
	std::shared_ptr<std::vector<uint8_t>> m_indexData;
	std::shared_ptr<std::vector<umath::VertexWeight>> m_vertexWeights;
	std::shared_ptr<std::vector<umath::VertexWeight>> m_extendedVertexWeights;
	udm::PProperty m_extensions = nullptr;
	Vector3 m_min;
	Vector3 m_max;
	GeometryType m_geometryType = GeometryType::Triangles;
	pragma::model::IndexType m_indexType = pragma::model::IndexType::UInt16;
	uint32_t m_referenceId = std::numeric_limits<uint32_t>::max();
	util::Uuid m_uuid;
	std::string m_name;
	umath::ScaledTransform m_pose = umath::ScaledTransform {};
	void ClipAgainstPlane(const Vector3 &n, double d, ModelSubMesh &clippedMesh, const std::vector<Mat4> *boneMatrices = nullptr, ModelSubMesh *clippedCoverMesh = nullptr);
};

REGISTER_BASIC_ARITHMETIC_OPERATORS(ModelSubMesh::ShareMode)

class DLLNETWORK ModelMesh : public std::enable_shared_from_this<ModelMesh> {
  public:
	ModelMesh();
	ModelMesh(const ModelMesh &other);
	ModelMesh &operator=(const ModelMesh &) = delete;
	bool operator==(const ModelMesh &other) const;
	bool operator!=(const ModelMesh &other) const;
	bool IsEqual(const ModelMesh &other) const;
	void Centralize();
	const Vector3 &GetCenter() const;
	void SetCenter(const Vector3 &center);
	virtual void AddSubMesh(const std::shared_ptr<ModelSubMesh> &subMesh);
	std::vector<std::shared_ptr<ModelSubMesh>> &GetSubMeshes();
	uint32_t GetVertexCount() const;
	uint32_t GetIndexCount() const;
	uint32_t GetTriangleCount() const;
	uint32_t GetSubMeshCount() const;
	virtual void Update(ModelUpdateFlags flags = ModelUpdateFlags::AllData);
	void GetBounds(Vector3 &min, Vector3 &max) const;
	void Rotate(const Quat &rot);
	void Translate(const Vector3 &t);
	void Merge(const ModelMesh &other);
	void Scale(const Vector3 &scale);
	void Mirror(pragma::Axis axis);
	virtual std::shared_ptr<ModelMesh> Copy() const;

	uint32_t GetReferenceId() const;
	void SetReferenceId(uint32_t refId);
  protected:
	Vector3 m_min;
	Vector3 m_max;
	uint32_t m_numVerts;
	uint32_t m_numIndices;
	Vector3 m_center;
	std::vector<std::shared_ptr<ModelSubMesh>> m_subMeshes;
	uint32_t m_referenceId = std::numeric_limits<uint32_t>::max();
};

DLLNETWORK std::ostream &operator<<(std::ostream &out, const ModelSubMesh &o);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const ModelMesh &o);

namespace pragma::model {
	struct DLLNETWORK QuadCreateInfo {
		QuadCreateInfo() = default;
		float size = 0.f;
	};
	DLLNETWORK void create_quad(ModelSubMesh &mesh, const QuadCreateInfo &createInfo);
	DLLNETWORK std::shared_ptr<ModelSubMesh> create_quad(Game &game, const QuadCreateInfo &createInfo);

	struct DLLNETWORK BoxCreateInfo {
		BoxCreateInfo() = default;
		BoxCreateInfo(const Vector3 &min, const Vector3 &max);
		Vector3 min;
		Vector3 max;
	};
	DLLNETWORK void create_box(ModelSubMesh &mesh, const BoxCreateInfo &createInfo);
	DLLNETWORK std::shared_ptr<ModelSubMesh> create_box(Game &game, const BoxCreateInfo &createInfo);

	struct DLLNETWORK SphereCreateInfo {
		SphereCreateInfo() = default;
		SphereCreateInfo(const Vector3 &origin, float radius);
		Vector3 origin;
		float radius = 0.f;
		uint32_t recursionLevel = 1;
	};
	DLLNETWORK void create_sphere(ModelSubMesh &mesh, const SphereCreateInfo &createInfo);
	DLLNETWORK std::shared_ptr<ModelSubMesh> create_sphere(Game &game, const SphereCreateInfo &createInfo);

	struct DLLNETWORK CylinderCreateInfo {
		CylinderCreateInfo() = default;
		CylinderCreateInfo(float radius, float length);
		float radius = 0.f;
		float length = 0.f;
		uint32_t segmentCount = 12;
	};
	DLLNETWORK void create_cylinder(ModelSubMesh &mesh, const CylinderCreateInfo &createInfo);
	DLLNETWORK std::shared_ptr<ModelSubMesh> create_cylinder(Game &game, const CylinderCreateInfo &createInfo);

	struct DLLNETWORK ConeCreateInfo {
		ConeCreateInfo() = default;
		ConeCreateInfo(umath::Degree angle, float length);
		ConeCreateInfo(float startRadius, float length, float endRadius);
		float length = 0.f;
		uint32_t segmentCount = 12;
		float startRadius = 0.f;
		float endRadius = 0.f;
	};
	DLLNETWORK void create_cone(ModelSubMesh &mesh, const ConeCreateInfo &createInfo);
	DLLNETWORK std::shared_ptr<ModelSubMesh> create_cone(Game &game, const ConeCreateInfo &createInfo);

	struct DLLNETWORK EllipticConeCreateInfo : public ConeCreateInfo {
		EllipticConeCreateInfo() = default;
		EllipticConeCreateInfo(umath::Degree angleX, umath::Degree angleY, float length);
		EllipticConeCreateInfo(float startRadiusX, float startRadiusY, float length, float endRadiusX, float endRadiusY);
		float startRadiusY = 0.f;
		float endRadiusY = 0.f;
	};
	DLLNETWORK void create_elliptic_cone(ModelSubMesh &mesh, const EllipticConeCreateInfo &createInfo);
	DLLNETWORK std::shared_ptr<ModelSubMesh> create_elliptic_cone(Game &game, const EllipticConeCreateInfo &createInfo);

	struct DLLNETWORK CircleCreateInfo {
		CircleCreateInfo() = default;
		CircleCreateInfo(float radius, bool doubleSided = true);
		float radius = 0.f;
		bool doubleSided = true;
		uint32_t segmentCount = 36;
		umath::Degree totalAngle = 360.f;
	};
	DLLNETWORK void create_circle(ModelSubMesh &mesh, const CircleCreateInfo &createInfo);
	DLLNETWORK std::shared_ptr<ModelSubMesh> create_circle(Game &game, const CircleCreateInfo &createInfo);

	struct DLLNETWORK RingCreateInfo {
		RingCreateInfo() = default;
		RingCreateInfo(float innerRadius, float outerRadius, bool doubleSided = true);
		std::optional<float> innerRadius {};
		float outerRadius = 0.f;
		bool doubleSided = true;
		uint32_t segmentCount = 36;
		umath::Degree totalAngle = 360.f;
	};
	DLLNETWORK void create_ring(ModelSubMesh &mesh, const RingCreateInfo &createInfo);
	DLLNETWORK std::shared_ptr<ModelSubMesh> create_ring(Game &game, const RingCreateInfo &createInfo);
};

#endif
