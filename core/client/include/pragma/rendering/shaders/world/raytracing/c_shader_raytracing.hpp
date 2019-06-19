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
			uint32_t width;
			uint32_t height;
			float fov;
		};
#pragma pack(pop)

		ShaderRayTracing(prosper::Context &context,const std::string &identifier);
		bool Compute(
			const PushConstants &pushConstants,
			Anvil::DescriptorSet &descSetOutputImage,Anvil::DescriptorSet &descSetGameScene,
			Anvil::DescriptorSet &descSetCamera,Anvil::DescriptorSet &descSetLightSources,
			uint32_t workGroupsX,uint32_t workGroupsY
		);

		bool ComputeTest();
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
