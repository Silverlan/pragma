// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.ssao_blur;

export import pragma.prosper;

export namespace pragma {
	namespace shaderSSAOBlur {
		using namespace prosper::shaderBaseImageProcessing;
	}
	class DLLCLIENT ShaderSSAOBlur : public prosper::ShaderBaseImageProcessing {
	  public:
		ShaderSSAOBlur(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};
