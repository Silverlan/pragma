#ifndef __C_SHADER_LIGHT_CONE_HPP__
#define __C_SHADER_LIGHT_CONE_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderLightCone
		: public ShaderTextured3DBase
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_MAP;

#pragma pack(push,1)
		struct PushConstants
		{
			float coneLength;
		};
#pragma pack(pop)

		ShaderLightCone(prosper::Context &context,const std::string &identifier);
		virtual bool BindSceneCamera(const Scene &scene,bool bView) override;
	protected:
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual void InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"

namespace Shader
{
	class DLLCLIENT LightCone
		: public TexturedBase3D
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			DepthMap = umath::to_integral(TexturedBase3D::DescSet::ShadowCubeMaps) +1
		};
	protected:
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void BindMaterialParameters(Material *mat) override;
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
	public:
		LightCone();
		virtual void BindMaterial(Material *mat) override;
		using TexturedBase3D::BeginDraw;
	};
};
#endif
#endif