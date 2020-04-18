#ifndef __C_GLTF_WRITER_HPP__
#define __C_GLTF_WRITER_HPP__

#include "pragma/asset/c_util_model.hpp"
#include <cinttypes>
#include <string>
#include <unordered_map>
#include <mathutil/uvec.h>
#include <tiny_gltf.h>

class Model;
class ModelSubMesh;
class MeshVertexFrame;
class Flex;
namespace pragma::asset
{
	class GLTFWriter
	{
	public:
		static bool Export(::Model &mdl,const ModelExportInfo &exportInfo,std::string &outErrMsg,const std::string *optModelName);
		static bool Export(::Model &mdl,const std::string &animName,const ModelExportInfo &exportInfo,std::string &outErrMsg,const std::string *optModelName);
	private:
		GLTFWriter(::Model &mdl,const ModelExportInfo &exportInfo,const std::optional<std::string> &animName);
		using BufferIndex = uint32_t;
		using BufferViewIndex = uint32_t;
		struct BufferIndices
		{
			static constexpr uint32_t Count = 4;
			BufferIndex indices = std::numeric_limits<uint32_t>::max();
			BufferIndex vertices = std::numeric_limits<uint32_t>::max();
			BufferIndex skin = std::numeric_limits<uint32_t>::max();
			BufferIndex inverseBindMatrices = std::numeric_limits<uint32_t>::max();
		};
		struct BufferViewIndices
		{
			static constexpr uint32_t Count = 7;
			BufferViewIndex indices = std::numeric_limits<uint32_t>::max();
			BufferViewIndex positions = std::numeric_limits<uint32_t>::max();
			BufferViewIndex normals = std::numeric_limits<uint32_t>::max();
			BufferViewIndex texCoords = std::numeric_limits<uint32_t>::max();
			BufferViewIndex joints = std::numeric_limits<uint32_t>::max();
			BufferViewIndex weights = std::numeric_limits<uint32_t>::max();
			BufferViewIndex inverseBindMatrices = std::numeric_limits<uint32_t>::max();
		};

		struct MorphSet
		{
			std::string name;
			MeshVertexFrame *frame = nullptr;
			uint32_t flexId = std::numeric_limits<uint32_t>::max();
		};

		struct GLTFVertexWeight
		{
			std::array<uint16_t,4> joints = {0,0,0,0};
			std::array<float,4> weights = {0.f,0.f,0.f,0.f};
		};
		Vector3 TransformPos(const Vector3 &v) const;

		bool Export(std::string &outErrMsg,const std::string *optModelName=nullptr);
		void WriteSkeleton();
		void WriteAnimations();
		void WriteMorphTargets(ModelSubMesh &mesh,tinygltf::Mesh &gltfMesh,tinygltf::Primitive &primitive,uint32_t nodeIdx);
		void WriteMaterials();
		void GenerateAO();
		uint32_t AddBufferView(const std::string &name,BufferIndex bufferIdx,uint64_t byteOffset,uint64_t byteLength,std::optional<uint64_t> byteStride);
		tinygltf::Buffer &AddBuffer(const std::string &name,uint32_t *optOutBufIdx=nullptr);
		uint32_t AddAccessor(const std::string &name,int componentType,int type,uint64_t byteOffset,uint64_t count,BufferViewIndex bufferViewIdx);

		uint32_t AddNode(const std::string &name,bool isRootNode);
		void InitializeMorphSets();
		void MergeSplitMeshes();
		tinygltf::Scene &GetScene();

		bool IsSkinned() const;
		bool IsAnimated() const;
		bool ShouldExportMeshes() const;

		pragma::asset::ModelExportInfo m_exportInfo {};
		std::string m_exportPath;
		std::optional<std::string> m_animName {};
		BufferIndices m_bufferIndices {};
		BufferViewIndices m_bufferViewIndices {};

		Model &m_model;
		std::vector<std::shared_ptr<ModelSubMesh>> m_meshes {};
		std::unordered_map<ModelSubMesh*,std::vector<MorphSet>> m_meshMorphSets {};
		std::unordered_map<ModelSubMesh*,uint32_t> m_meshesWithMorphTargets {};
		std::unordered_map<uint32_t,uint32_t> m_boneIdxToNodeIdx {};
		tinygltf::Model m_gltfMdl = {};
		int32_t m_skinIdx = -1;
	};
};

#endif
