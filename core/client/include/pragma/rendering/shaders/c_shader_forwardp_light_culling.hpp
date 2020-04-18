#ifndef __C_SHADER_FORWARDP_LIGHT_CULLING_HPP__
#define __C_SHADER_FORWARDP_LIGHT_CULLING_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderForwardPLightCulling
		: public prosper::ShaderCompute
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;

		static uint32_t TILE_SIZE;

		enum class CameraBinding : uint32_t
		{
			Camera = 0u,
			RenderSettings
		};

		enum class LightBinding : uint32_t
		{
			LightBuffers = 0u,
			TileVisLightIndexBuffer,
			ShadowData,
			VisLightIndexBuffer,
			DepthMap
		};

#pragma pack(push,1)
		struct PushConstants
		{
			uint32_t numLights;
			uint32_t sceneFlag;
		};
#pragma pack(pop)

		ShaderForwardPLightCulling(prosper::Context &context,const std::string &identifier);
		bool Compute(
			Anvil::DescriptorSet &descSetLights,Anvil::DescriptorSet &descSetCamera,uint32_t workGroupsX,uint32_t workGroupsY,uint32_t lightCount,
			uint32_t sceneIndex
		);
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "shadersystem.h"

namespace Shader
{
	class DLLCLIENT ForwardPLightCulling
		: public Base
	{
	public:
		ForwardPLightCulling();
		void Compute(const Vulkan::CommandBuffer &computeCmd,const Vulkan::DescriptorSet &descSetLights,const Vulkan::DescriptorSet &descSetCamera,uint32_t workGroupsX,uint32_t workGroupsY,uint32_t lightCount); // Currently no barriers in place; Returned color might be from a previous call
		static Vulkan::DescriptorSet CreateLightDescriptorSet();

		enum class DescSet : uint32_t
		{
			LightBuffers = 0,
			TileVisLightIndexBuffer = LightBuffers,
			ShadowData = LightBuffers,
			VisLightIndexBuffer = ShadowData,
			DepthMap = VisLightIndexBuffer,

			Camera = DepthMap +1,
			RenderSettings = Camera,
		};
		enum class Binding : uint32_t
		{
			LightBuffers = 0,
			TileVisLightIndexBuffer = LightBuffers +1,
			ShadowData = TileVisLightIndexBuffer +1,
			VisLightIndexBuffer = ShadowData +1,
			DepthMap = VisLightIndexBuffer +1,

			Camera = 0,
			RenderSettings = Camera +1,
		};
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	};
};
#endif
#endif