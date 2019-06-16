#ifndef __C_SHADER_RAYTRACING_HPP__
#define __C_SHADER_RAYTRACING_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderRayTracing
		: public prosper::ShaderCompute
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_IMAGE_OUTPUT;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_BUFFERS;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_GAME_SCENE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;

		enum class GameSceneBinding : uint32_t
		{
			TextureArray=0,
			MaterialInfos,
			SubMeshInfos,
			EntityInstanceData,
			BoneMatrices,

			VertexBuffer,
			IndexBuffer,
			VertexWeightBuffer,
			AlphaBuffer
		};

#pragma pack(push,1)
		struct PushConstants
		{
			uint32_t numMeshes;
			uint32_t lightCount;
		};
#pragma pack(pop)

		ShaderRayTracing(prosper::Context &context,const std::string &identifier);
		bool Compute(
			Anvil::DescriptorSet &descSetImage,Anvil::DescriptorSet &descSetBuffers,
			Anvil::DescriptorSet &descSetMat,Anvil::DescriptorSet &descSetCamera,
			uint32_t workGroupsX,uint32_t workGroupsY
		);

		void Test();
		bool ComputeTest();
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
