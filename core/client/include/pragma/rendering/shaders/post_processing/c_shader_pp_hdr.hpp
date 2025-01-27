/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PP_HDR_HPP__
#define __C_SHADER_PP_HDR_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"
#include "pragma/rendering/c_settings.hpp"

namespace pragma {
	namespace rendering {
		enum class ToneMapping : uint32_t;
	};
	class DLLCLIENT ShaderPPHDR : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::Format RENDER_PASS_FORMAT;
		static prosper::Format RENDER_PASS_FORMAT_HDR;

		enum class Pipeline : uint32_t {
			LDR = 0,
			HDR,

			Count
		};

		enum class TextureBinding : uint32_t {
			Texture = 0u,
			Bloom,
		};

#pragma pack(push, 1)
		struct PushConstants {
			float exposure;
			float bloomScale;
			rendering::ToneMapping toneMapping;
			uint32_t flipVertically;
		};
#pragma pack(pop)

		ShaderPPHDR(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, pragma::rendering::ToneMapping toneMapping, float exposure, float bloomScale, bool flipVertically = false) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};

#endif
