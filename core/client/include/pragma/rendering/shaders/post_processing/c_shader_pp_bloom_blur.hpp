/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __C_SHADER_PP_BLOOM_BLUR_HPP__
#define __C_SHADER_PP_BLOOM_BLUR_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_blur.hpp>

namespace pragma {
	struct BloomPipelineInfo {
		BloomPipelineInfo() = delete;
		BloomPipelineInfo(uint32_t radius, double sigma);
		const uint32_t radius;
		const double sigma;
		std::optional<uint32_t> pipelineIdx;
	};
	class DLLCLIENT ShaderPPBloomBlurBase : public prosper::ShaderBlurBase {
	  public:
		static constexpr uint32_t MAX_SAMPLE_COUNT = 15;
		static constexpr uint32_t DEFAULT_RADIUS = 6;
		static constexpr double DEFAULT_SIGMA = 10;
		ShaderPPBloomBlurBase(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader);

		std::shared_ptr<BloomPipelineInfo> AddPipeline(uint32_t radius, double sigma);
	  protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void OnInitializePipelines() override;
		std::vector<std::weak_ptr<BloomPipelineInfo>> m_pipelineInfos;
		std::shared_ptr<BloomPipelineInfo> m_defaultPipeline;
	};

	/////////////////////////

	class DLLCLIENT ShaderPPBloomBlurH : public ShaderPPBloomBlurBase {
	  public:
		ShaderPPBloomBlurH(prosper::IPrContext &context, const std::string &identifier);
		~ShaderPPBloomBlurH();
	};

	/////////////////////////

	class DLLCLIENT ShaderPPBloomBlurV : public ShaderPPBloomBlurBase {
	  public:
		ShaderPPBloomBlurV(prosper::IPrContext &context, const std::string &identifier);
		~ShaderPPBloomBlurV();
	};
};

#endif
