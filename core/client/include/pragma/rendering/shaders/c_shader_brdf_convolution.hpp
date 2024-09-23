/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_BRDF_CONVOLUTION_HPP__
#define __C_SHADER_BRDF_CONVOLUTION_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {
	class Texture;
};
namespace pragma {
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

#endif
