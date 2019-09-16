#ifndef __C_SHADER_RAYTRACING_HPP__
#define __C_SHADER_RAYTRACING_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_raytracing_component.hpp"
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
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_IBL;

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

		enum class RenderFlags : uint32_t
		{
			None = 0u,
			RenderWorld = umath::to_integral(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeWorld),
			RenderView = umath::to_integral(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeView),
			RenderSkybox = umath::to_integral(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeSkybox),
			RenderWater = umath::to_integral(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeWater),
			NoIBL = RenderWater<<1u
		};

#pragma pack(push,1)
		struct PushConstants
		{
			uint32_t numMeshes;
			uint32_t lightCount;
			uint32_t width;
			uint32_t height;
			float fov;
			uint32_t pxOffset; // First 16 bits = x-offset, last 16 bits = y-offset of the target image
			RenderFlags renderFlags;
		};
#pragma pack(pop)

		ShaderRayTracing(prosper::Context &context,const std::string &identifier);
		bool Compute(
			const PushConstants &pushConstants,
			Anvil::DescriptorSet &descSetOutputImage,Anvil::DescriptorSet &descSetGameScene,
			Anvil::DescriptorSet &descSetCamera,Anvil::DescriptorSet &descSetLightSources,
			Anvil::DescriptorSet *descSetIBL,
			uint32_t workGroupsX,uint32_t workGroupsY
		);
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderRayTracing::RenderFlags)

#endif
