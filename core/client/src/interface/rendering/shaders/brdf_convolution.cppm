// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.brdf_convolution;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderBRDFConvolution : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PREPASS;

		ShaderBRDFConvolution(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::Texture> CreateBRDFConvolutionMap(uint32_t resolution);
	  protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
