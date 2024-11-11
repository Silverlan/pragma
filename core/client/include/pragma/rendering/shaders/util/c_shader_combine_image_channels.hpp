/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __C_SHADER_COMBINE_IMAGE_CHANNELS_HPP__
#define __C_SHADER_COMBINE_IMAGE_CHANNELS_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {
	class Image;
	class Texture;
};
namespace pragma {
	class DLLCLIENT ShaderCombineImageChannels : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		enum class TextureBinding : uint32_t {
			ChannelR,
			ChannelG,
			ChannelB,
			ChannelA,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			uint32_t redChannel = 0;
			uint32_t greenChannel = 1;
			uint32_t blueChannel = 2;
			uint32_t alphaChannel = 3;
		};
#pragma pack(pop)

		ShaderCombineImageChannels(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::Texture> CombineImageChannels(prosper::IPrContext &context, prosper::Texture &channelR, prosper::Texture &channelG, prosper::Texture &channelB, prosper::Texture &channelA, const PushConstants &pushConstants);
	  protected:
		std::shared_ptr<prosper::RenderTarget> CreateRenderTarget(prosper::IPrContext &context, prosper::Extent2D extents) const;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
