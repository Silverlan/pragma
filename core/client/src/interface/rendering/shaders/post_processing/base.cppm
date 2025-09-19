// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

export module pragma.client.rendering.shaders:base;

export namespace pragma {
	class DLLCLIENT ShaderPPBase : public prosper::ShaderBaseImageProcessing {
	  public:
		ShaderPPBase(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader);
		ShaderPPBase(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader);
	  protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};
