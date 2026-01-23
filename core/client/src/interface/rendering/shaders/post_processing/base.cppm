// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.base;

export import pragma.prosper;

export namespace pragma {
	namespace shaderPPBase {
		using namespace prosper::shaderBaseImageProcessing;
	}
	class DLLCLIENT ShaderPPBase : public prosper::ShaderBaseImageProcessing {
	  public:
		ShaderPPBase(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader);
		ShaderPPBase(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader);
	  protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};
