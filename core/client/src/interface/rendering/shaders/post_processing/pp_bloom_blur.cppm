// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_blur.hpp>

export module pragma.client.rendering.shaders:pp_bloom_blur;

import pragma.client.rendering;

export namespace pragma {
	class DLLCLIENT ShaderPPBloomBlurBase : public prosper::ShaderBlurBase {
	  public:
		static constexpr uint32_t MAX_SAMPLE_COUNT = 15;
		static constexpr uint32_t DEFAULT_RADIUS = ControlledBlurSettings::DEFAULT_RADIUS;
		static constexpr double DEFAULT_SIGMA = ControlledBlurSettings::DEFAULT_SIGMA;
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
