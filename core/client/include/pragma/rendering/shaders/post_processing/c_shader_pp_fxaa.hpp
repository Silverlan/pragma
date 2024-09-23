/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PP_FXAA_HPP__
#define __C_SHADER_PP_FXAA_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"

namespace pragma {
	class DLLCLIENT ShaderPPFXAA : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		enum class TextureBinding : uint32_t { SceneTexturePostToneMapping = 0, SceneTextureHdr };

#pragma pack(push, 1)
		struct DLLCLIENT PushConstants {
			// Required due to gcc bug (see https://stackoverflow.com/q/46866686)
			PushConstants() {};
			~PushConstants() {};

			float subPixelAliasingRemoval = 0.75f;
			float edgeThreshold = 0.166f;
			float minEdgeThreshold = 0.0833f;
		};
#pragma pack(pop)

		ShaderPPFXAA(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, const PushConstants &pushConstants = PushConstants {}) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};

#endif
