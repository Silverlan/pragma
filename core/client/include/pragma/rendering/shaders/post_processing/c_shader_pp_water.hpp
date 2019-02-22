#ifndef __C_SHADER_PP_WATER_HPP__
#define __C_SHADER_PP_WATER_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"

namespace pragma
{
	class DLLCLIENT ShaderPPWater
		: public ShaderPPBase
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_BUFFER;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_REFRACTION_MAP;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_FOG;

#pragma pack(push,1)
		struct PushConstants
		{
			Vector4 clipPlane;
		};
#pragma pack(pop)

		ShaderPPWater(prosper::Context &context,const std::string &identifier);
		std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat);
		bool BindRefractionMaterial(CMaterial &mat);
		bool Draw(
			Anvil::DescriptorSet &descSetTexture,Anvil::DescriptorSet &descSetDepth,
			Anvil::DescriptorSet &descSetCamera,Anvil::DescriptorSet &descSetTime,
			Anvil::DescriptorSet &descSetFog,const Vector4 &clipPlane
		);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "shadersystem.h"
#include "shader_screen.h"

namespace Shader
{
	class DLLCLIENT PPWater
		: public Screen
	{
	public:
		PPWater();

		enum class DescSet : uint32_t
		{
			Texture = 0u,

			DepthBuffer = Texture +1u,
			
			RefractionMap = DepthBuffer +1u,

			Camera = RefractionMap +1u,

			Time = Camera +1u,

			Fog = Time +1u
		};

		enum class Binding : uint32_t
		{
			Texture = 0u,

			DepthBuffer = 0u,

			RefractionMap = 1u,

			Camera = 0u,

			Time = 1u,

			Fog = 0u,
		};
		void Draw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descTexture,const Vulkan::DescriptorSet &descDepth,const Vulkan::DescriptorSet &descFog,const Vector4 &clipPlane);
		void BindMaterial(const Vulkan::CommandBufferObject *drawCmd,const CMaterial &mat);
		void BindScene(const Vulkan::CommandBufferObject *drawCmd,const Scene &scene);
		using Screen::BeginDraw;
	protected:
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
	};
};
#endif
#endif
