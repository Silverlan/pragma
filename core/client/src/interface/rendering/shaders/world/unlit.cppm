// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <mathutil/uvec.h>

export module pragma.client:rendering.shaders.world_unlit;

import :rendering.shaders.textured;

export namespace pragma {
	class DLLCLIENT ShaderUnlit : public ShaderGameWorldLightingPass {
	  public:
		ShaderUnlit(prosper::IPrContext &context, const std::string &identifier);
	};
};
