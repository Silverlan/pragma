#ifndef __C_SHADER_PP_HDR_HPP__
#define __C_SHADER_PP_HDR_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"

namespace pragma
{
	namespace rendering
	{
		enum class ToneMapping : uint32_t;
	};
	class DLLCLIENT ShaderPPHDR
		: public ShaderPPBase
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static Anvil::Format RENDER_PASS_FORMAT;

		enum class TextureBinding : uint32_t
		{
			Texture = 0u,
			Bloom,
			Glow
		};
#pragma pack(push,1)
		struct PushConstants
		{
			float exposure;
			float bloomScale;
			float glowScale;
			rendering::ToneMapping toneMapping;
		};
#pragma pack(pop)

		ShaderPPHDR(prosper::Context &context,const std::string &identifier);
		bool Draw(Anvil::DescriptorSet &descSetTexture,float exposure,float bloomScale,float glowScale);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "shadersystem.h"
#include "shader_screen.h"

namespace Shader
{
	class DLLCLIENT PPHDR
		: public Screen
	{
	public:
		PPHDR();

		enum class DescSet : uint32_t
		{
			Texture = 0u,
			Bloom = Texture +1u,
			Glow = Bloom
		};

		enum class Binding : uint32_t
		{
			Texture = 0u,
			Bloom = 0u,
			Glow = Bloom +1u
		};
		void Draw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descTexture,const Vulkan::DescriptorSet &descBloomGlow,float exposure,float bloomScale,float glowScale);
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	};
};
#endif
#endif
