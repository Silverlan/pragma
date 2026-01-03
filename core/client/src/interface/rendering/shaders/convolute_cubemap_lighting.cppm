// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.convolute_cubemap_lighting;

export import :rendering.shaders.cubemap;

export namespace pragma {
	class DLLCLIENT ShaderConvoluteCubemapLighting : public ShaderCubemap {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CUBEMAP_TEXTURE;

		ShaderConvoluteCubemapLighting(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::Texture> ConvoluteCubemapLighting(prosper::Texture &cubemap, uint32_t resolution);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
