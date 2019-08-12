#ifndef __C_SHADER_CONVOLUTE_CUBEMAP_LIGHTING_HPP
#define __C_SHADER_CONVOLUTE_CUBEMAP_LIGHTING_HPP

#include "pragma/rendering/shaders/c_shader_base_cubemap.hpp"

namespace prosper {class Texture;};
namespace pragma
{
	class DLLCLIENT ShaderConvoluteCubemapLighting
		: public ShaderCubemap
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CUBEMAP_TEXTURE;

		ShaderConvoluteCubemapLighting(prosper::Context &context,const std::string &identifier);
		std::shared_ptr<prosper::Texture> ConvoluteCubemapLighting(prosper::Texture &cubemap,uint32_t resolution);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};
};

#endif
