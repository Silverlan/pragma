// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

export module pragma.client.rendering.shaders:brdf_convolution;

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
