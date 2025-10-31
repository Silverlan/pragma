// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.shaders.ssao_blur;

export namespace pragma {
	class DLLCLIENT ShaderSSAOBlur : public prosper::ShaderBaseImageProcessing {
	  public:
		ShaderSSAOBlur(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};
