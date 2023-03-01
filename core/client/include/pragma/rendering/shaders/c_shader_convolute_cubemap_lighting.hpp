/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_CONVOLUTE_CUBEMAP_LIGHTING_HPP
#define __C_SHADER_CONVOLUTE_CUBEMAP_LIGHTING_HPP

#include "pragma/rendering/shaders/c_shader_base_cubemap.hpp"

namespace prosper {
	class Texture;
};
namespace pragma {
	class DLLCLIENT ShaderConvoluteCubemapLighting : public ShaderCubemap {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CUBEMAP_TEXTURE;

		ShaderConvoluteCubemapLighting(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::Texture> ConvoluteCubemapLighting(prosper::Texture &cubemap, uint32_t resolution);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};

#endif
