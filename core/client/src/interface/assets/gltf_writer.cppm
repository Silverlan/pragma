// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <tiny_gltf.h>

export module pragma.client:assets.gltf_writer;

import :assets.import_export;

export namespace pragma::asset {
	class GLTFWriter {
	  public:
		struct ModelDesc {
			ModelDesc(Model &mdl, const math::Transform &pose = {}) : model {mdl}, pose {pose} {}
			Model &model;
			math::Transform pose;
		};
		struct LightSource {
			enum class Type : uint8_t { Point = 0, Spot, Directional };
			std::string name;
			Color color;
			union {
				Candela luminousIntensity = 1.f;
				Lux illuminance;
			};
			Type type = Type::Point;
			std::optional<float> range = {};
			math::Transform pose {};

			// Spot lights
			math::Fraction blendFraction = 0.f;
			math::Degree outerConeAngle = 90.f;
		};
		struct Camera {
			enum class Type : uint8_t { Perspective = 0, Orthographic };
			std::string name;
			Type type = Type::Perspective;
			float aspectRatio = 1.f;
			math::Degree vFov = 90.f;
			float zNear = 0.01f;
			float zFar = 1'000.f;

			math::Transform pose {};
		};
		using ModelCollection = std::vector<ModelDesc>;
		using LightSourceList = std::vector<LightSource>;
		using CameraList = std::vector<Camera>;
		struct SceneDesc {
			ModelCollection modelCollection {};
			LightSourceList lightSources {};
			CameraList cameras {};
		};

		static bool Export(const SceneDesc &sceneDesc, const std::string &outputFileName, const ModelExportInfo &exportInfo, std::string &outErrMsg, std::string *optOutPath = nullptr);
		static bool Export(const SceneDesc &sceneDesc, const std::string &outputFileName, const std::string &animName, const ModelExportInfo &exportInfo, std::string &outErrMsg, std::string *optOutPath = nullptr);

		static bool Export(Model &model, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &outputFileName = {}, std::string *optOutPath = nullptr);
		static bool Export(Model &model, const std::string &animName, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &outputFileName = {}, std::string *optOutPath = nullptr);
	  private:
		GLTFWriter(const SceneDesc &sceneDesc, const ModelExportInfo &exportInfo, const std::optional<std::string> &animName);
		using BufferIndex = uint32_t;
		using BufferViewIndex = uint32_t;
		struct BufferIndices {
			static constexpr uint32_t Count = 5;
			BufferIndex indices = std::numeric_limits<uint32_t>::max();
			BufferIndex vertices = std::numeric_limits<uint32_t>::max();
			BufferIndex uvSets = std::numeric_limits<uint32_t>::max();
			BufferIndex skin = std::numeric_limits<uint32_t>::max();
			BufferIndex inverseBindMatrices = std::numeric_limits<uint32_t>::max();
		};
		struct BufferViewIndices {
			static constexpr uint32_t Count = 8;
			BufferViewIndex indices = std::numeric_limits<uint32_t>::max();
			BufferViewIndex positions = std::numeric_limits<uint32_t>::max();
			BufferViewIndex normals = std::numeric_limits<uint32_t>::max();
			BufferViewIndex texCoords = std::numeric_limits<uint32_t>::max();
			BufferViewIndex uvSets = std::numeric_limits<uint32_t>::max();
			BufferViewIndex joints = std::numeric_limits<uint32_t>::max();
			BufferViewIndex weights = std::numeric_limits<uint32_t>::max();
			BufferViewIndex inverseBindMatrices = std::numeric_limits<uint32_t>::max();
		};
		using ExportMeshList = std::vector<std::shared_ptr<geometry::ModelSubMesh>>;
		struct ModelExportData {
			ModelExportData(Model &model) : model {model} {}

			Model &model;
			std::vector<math::Transform> instances {};

			ExportMeshList exportMeshes;
			uint64_t indexCount = 0;
			uint64_t vertCount = 0;
			int32_t skinIndex = -1;
		};
		using UniqueModelExportList = std::vector<ModelExportData>;

		struct MorphSet {
			std::string name;
			animation::MeshVertexFrame *frame = nullptr;
			uint32_t flexId = std::numeric_limits<uint32_t>::max();
		};

		struct GLTFVertexWeight {
			std::array<uint16_t, 4> joints = {0, 0, 0, 0};
			std::array<float, 4> weights = {0.f, 0.f, 0.f, 0.f};
		};
		Vector3 TransformPos(const Vector3 &v) const;

		bool Export(std::string &outErrMsg, const std::string &outputFileName, std::string *optOutPath = nullptr);
		void GenerateUniqueModelExportList();
		void WriteSkeleton(ModelExportData &mdlData);
		void WriteAnimations(Model &mdl);
		void WriteMorphTargets(geometry::ModelSubMesh &mesh, tinygltf::Mesh &gltfMesh, tinygltf::Primitive &primitive, const std::vector<uint32_t> &nodeIndices);
		void WriteMaterials();
		void WriteLightSources();
		void ToGLTFPose(const math::Transform &pose, std::vector<double> &outPos, std::vector<double> &outRot) const;
		void GenerateAO(Model &mdl);
		uint32_t AddBufferView(const std::string &name, BufferIndex bufferIdx, uint64_t byteOffset, uint64_t byteLength, std::optional<uint64_t> byteStride);
		tinygltf::Buffer &AddBuffer(const std::string &name, uint32_t *optOutBufIdx = nullptr);
		uint32_t AddAccessor(const std::string &name, int componentType, int type, uint64_t byteOffset, uint64_t count, BufferViewIndex bufferViewIdx);

		uint32_t AddNode(const std::string &name, bool isRootNode);
		void InitializeMorphSets(Model &mdl);
		void MergeSplitMeshes(ExportMeshList &meshList);
		tinygltf::Scene &GetScene();

		bool IsSkinned(Model &mdl) const;
		bool IsAnimated(Model &mdl) const;
		bool ShouldExportMeshes() const;

		ModelExportInfo m_exportInfo {};
		std::string m_exportPath;
		std::optional<std::string> m_animName {};
		BufferIndices m_bufferIndices {};
		BufferViewIndices m_bufferViewIndices {};

		SceneDesc m_sceneDesc {};
		std::unordered_map<material::Material *, uint32_t> m_materialToGltfIndex {};
		UniqueModelExportList m_uniqueModelExportList {};
		std::unordered_map<geometry::ModelSubMesh *, std::vector<MorphSet>> m_meshMorphSets {};
		std::unordered_map<geometry::ModelSubMesh *, std::vector<uint32_t>> m_meshesWithMorphTargets {};
		std::unordered_map<uint32_t, uint32_t> m_boneIdxToNodeIdx {};
		tinygltf::Model m_gltfMdl = {};
	};
};
