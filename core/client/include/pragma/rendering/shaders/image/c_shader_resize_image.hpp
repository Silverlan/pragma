/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_RESIZE_IMAGE_HPP__
#define __C_SHADER_RESIZE_IMAGE_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace pragma {
	class DLLCLIENT ShaderResizeImage : public prosper::ShaderBaseImageProcessing {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			Vector4i iparam;
			Vector4 fparam;
		};
#pragma pack(pop)

		enum class Filter : uint32_t {
			Bicubic = 0,
			Lanczos,

			Count
		};

		enum class RenderPass : uint8_t {
			R8G8B8A8,
			R16G16B16A16,
			Count,
		};

		struct DLLCLIENT BicubicFilter {};

		struct DLLCLIENT LanczosFilter {
			enum class Scale : uint32_t { e2 = 2, e4 = 4 };
			Scale scale = Scale::e2;
		};

		ShaderResizeImage(prosper::IPrContext &context, const std::string &identifier);
		virtual ~ShaderResizeImage() override;
		bool RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTexture, const BicubicFilter &bicubicFilter, prosper::Format format = prosper::Format::R8G8B8A8_UNorm) const;
		bool RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTexture, const LanczosFilter &lanczosFilter, prosper::Format format = prosper::Format::R8G8B8A8_UNorm) const;
	  protected:
		Filter GetFilter(uint32_t pipelineIdx) const;
		RenderPass GetRenderPassType(uint32_t pipelineIdx) const;
		std::optional<RenderPass> GetRenderPassType(prosper::Format format) const;
		prosper::Format GetFormat(uint32_t pipelineIdx) const;
		uint32_t GetPipelineIndex(Filter filter, RenderPass renderPass) const;
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, const PushConstants &pushConstants) const;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
