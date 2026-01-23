// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:model.model_mesh;

export import :game.coordinate_system;
export import :game.enums;
export import :model.enums;
export import :types;
export import pragma.udm;

export {
	namespace pragma::math {
		DLLNETWORK void normalize_uv_coordinates(Vector2 &uv);
		template<typename TIndex>
		void compute_tangent_basis(std::vector<Vertex> &verts, const TIndex *indices, uint32_t numIndices);
	};

	namespace pragma::geometry {
		class DLLNETWORK ModelSubMesh : public std::enable_shared_from_this<ModelSubMesh> {
		  public:
			static constexpr std::uint32_t layout_version = 1;

			static constexpr auto PMESH_IDENTIFIER = "PMESH";
			static constexpr udm::Version PMESH_VERSION = 1;
			enum class ShareMode : uint32_t { None = 0, Vertices = 1, Alphas = 2, Triangles = 4, VertexWeights = 8, All = Vertices | Alphas | Triangles | VertexWeights };
			enum class GeometryType : uint8_t { Triangles = 0u, Lines, Points };
			using Index16 = uint16_t;
			using Index32 = uint32_t;
			static constexpr auto MAX_INDEX16 = std::numeric_limits<Index16>::max();
			static constexpr auto MAX_INDEX32 = std::numeric_limits<Index32>::max();
			static size_t size_of_index(IndexType it)
			{
				switch(it) {
				case IndexType::UInt16:
					return sizeof(Index16);
				case IndexType::UInt32:
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
			std::vector<math::Vertex> &GetVertices();
			std::vector<Vector2> &GetAlphas();
			std::vector<uint8_t> &GetIndexData();
			void GetIndices(std::vector<Index32> &outIndices) const;
			std::optional<Index32> GetIndex(uint32_t i) const;
			bool SetIndex(uint32_t i, Index32 idx);
			std::vector<math::VertexWeight> &GetVertexWeights();         // Vertex weights 0-3
			std::vector<math::VertexWeight> &GetExtendedVertexWeights(); // Vertex weights 0-7
			const std::vector<math::Vertex> &GetVertices() const { return const_cast<ModelSubMesh *>(this)->GetVertices(); }
			const std::vector<Vector2> &GetAlphas() const { return const_cast<ModelSubMesh *>(this)->GetAlphas(); }
			const std::vector<uint8_t> &GetIndexData() const { return const_cast<ModelSubMesh *>(this)->GetIndexData(); }
			const std::vector<math::VertexWeight> &GetVertexWeights() const { return const_cast<ModelSubMesh *>(this)->GetVertexWeights(); }
			const std::vector<math::VertexWeight> &GetExtendedVertexWeights() const { return const_cast<ModelSubMesh *>(this)->GetExtendedVertexWeights(); }
			void GetBounds(Vector3 &min, Vector3 &max) const;
			uint8_t GetAlphaCount() const;
			void SetAlphaCount(uint8_t numAlpha);
			uint32_t AddVertex(const math::Vertex &v);
			void AddTriangle(const math::Vertex &v1, const math::Vertex &v2, const math::Vertex &v3);
			void AddTriangle(uint32_t a, uint32_t b, uint32_t c);
			void AddIndex(Index32 index);
			void AddLine(uint32_t idx0, uint32_t idx1);
			void AddPoint(uint32_t idx);
			void ReserveIndices(size_t num);
			void ReserveVertices(size_t num);
			void Validate();
			virtual void Update(asset::ModelUpdateFlags flags = asset::ModelUpdateFlags::AllData);

			GeometryType GetGeometryType() const;
			void SetGeometryType(GeometryType type);

			IndexType GetIndexType() const;
			void SetIndexType(IndexType type);
			udm::Type GetUdmIndexType() const;
			void VisitIndices(auto vs)
			{
				auto &indexData = GetIndexData();
				switch(m_indexType) {
				case IndexType::UInt16:
					vs(reinterpret_cast<Index16 *>(indexData.data()), GetIndexCount());
					break;
				case IndexType::UInt32:
					vs(reinterpret_cast<Index32 *>(indexData.data()), GetIndexCount());
					break;
				}
			}
			void VisitIndices(auto vs) const
			{
				auto &indexData = GetIndexData();
				switch(m_indexType) {
				case IndexType::UInt16:
					vs(reinterpret_cast<const Index16 *>(indexData.data()), GetIndexCount());
					break;
				case IndexType::UInt32:
					vs(reinterpret_cast<const Index32 *>(indexData.data()), GetIndexCount());
					break;
				}
			}

			void SetVertex(uint32_t idx, const math::Vertex &v);
			void SetVertexPosition(uint32_t idx, const Vector3 &pos);
			void SetVertexNormal(uint32_t idx, const Vector3 &normal);
			void SetVertexUV(uint32_t idx, const Vector2 &uv);
			void SetVertexAlpha(uint32_t idx, const Vector2 &alpha);
			void SetVertexWeight(uint32_t idx, const math::VertexWeight &weight);

			const std::vector<Vector2> *GetUVSet(const std::string &name) const;
			std::vector<Vector2> *GetUVSet(const std::string &name);
			const std::unordered_map<std::string, std::vector<Vector2>> &GetUVSets() const;
			std::unordered_map<std::string, std::vector<Vector2>> &GetUVSets();
			std::vector<Vector2> &AddUVSet(const std::string &name);

			math::Vertex GetVertex(uint32_t idx) const;
			Vector3 GetVertexPosition(uint32_t idx) const;
			Vector3 GetVertexNormal(uint32_t idx) const;
			Vector2 GetVertexUV(uint32_t idx) const;
			Vector2 GetVertexAlpha(uint32_t idx) const;
			math::VertexWeight GetVertexWeight(uint32_t idx) const;
			void Optimize(double epsilon = math::VERTEX_EPSILON);
			void Rotate(const Quat &rot);
			void Translate(const Vector3 &t);
			void Transform(const math::ScaledTransform &pose);
			void Merge(const ModelSubMesh &other);
			void Scale(const Vector3 &scale);
			void Mirror(Axis axis);
			void ClipAgainstPlane(const Vector3 &n, double d, ModelSubMesh &clippedMeshA, ModelSubMesh &clippedMeshB, const std::vector<Mat4> *boneMatrices = nullptr, ModelSubMesh *clippedCoverMeshA = nullptr, ModelSubMesh *clippedCoverMeshB = nullptr);
			virtual std::shared_ptr<ModelSubMesh> Copy(bool fullCopy = false) const;
			std::shared_ptr<ModelSubMesh> Simplify(uint32_t targetVertexCount, double aggressiveness = 5.0, std::vector<uint64_t> *optOutNewVertexIndexToOriginalIndex = nullptr) const;

			void ApplyUVMapping(const Vector3 &nu, const Vector3 &nv, uint32_t w, uint32_t h, float ou, float ov, float su, float sv);
			void RemoveVertex(uint64_t idx);

			const math::ScaledTransform &GetPose() const;
			math::ScaledTransform &GetPose();
			void SetPose(const math::ScaledTransform &pose);

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
			std::vector<math::VertexWeight> &GetVertexWeightSet(uint32_t idx);
			const std::vector<math::VertexWeight> &GetVertexWeightSet(uint32_t idx) const;
			void ComputeTangentBasis();

			uint32_t m_skinTextureIndex;
			Vector3 m_center;
			std::shared_ptr<std::vector<math::Vertex>> m_vertices;
			std::shared_ptr<std::vector<Vector2>> m_alphas;
			std::shared_ptr<std::unordered_map<std::string, std::vector<Vector2>>> m_uvSets;
			uint8_t m_numAlphas;
			std::shared_ptr<std::vector<uint8_t>> m_indexData;
			std::shared_ptr<std::vector<math::VertexWeight>> m_vertexWeights;
			std::shared_ptr<std::vector<math::VertexWeight>> m_extendedVertexWeights;
			udm::PProperty m_extensions = nullptr;
			Vector3 m_min;
			Vector3 m_max;
			GeometryType m_geometryType = GeometryType::Triangles;
			IndexType m_indexType = IndexType::UInt16;
			uint32_t m_referenceId = std::numeric_limits<uint32_t>::max();
			util::Uuid m_uuid;
			std::string m_name;
			math::ScaledTransform m_pose = math::ScaledTransform {};
			void ClipAgainstPlane(const Vector3 &n, double d, ModelSubMesh &clippedMesh, const std::vector<Mat4> *boneMatrices = nullptr, ModelSubMesh *clippedCoverMesh = nullptr);
		};
		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::geometry::ModelSubMesh::ShareMode)

	namespace pragma::geometry {
		class DLLNETWORK geometry::ModelMesh : public std::enable_shared_from_this<ModelMesh> {
		public:
			static constexpr std::uint32_t layout_version = 1;

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
			virtual void Update(asset::ModelUpdateFlags flags = asset::ModelUpdateFlags::AllData);
			void GetBounds(Vector3 &min, Vector3 &max) const;
			void Rotate(const Quat &rot);
			void Translate(const Vector3 &t);
			void Merge(const ModelMesh &other);
			void Scale(const Vector3 &scale);
			void Mirror(Axis axis);
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
			ConeCreateInfo(math::Degree angle, float length);
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
			EllipticConeCreateInfo(math::Degree angleX, math::Degree angleY, float length);
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
			math::Degree totalAngle = 360.f;
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
			math::Degree totalAngle = 360.f;
		};
		DLLNETWORK void create_ring(ModelSubMesh &mesh, const RingCreateInfo &createInfo);
		DLLNETWORK std::shared_ptr<ModelSubMesh> create_ring(Game &game, const RingCreateInfo &createInfo);
	};
};
