// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:rendering.shaders.world_wireframe;

import :rendering.shaders.pbr;

export namespace pragma {
	class DLLCLIENT ShaderWireframe : public ShaderPBR {
	  public:
		ShaderWireframe(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
	};
};
