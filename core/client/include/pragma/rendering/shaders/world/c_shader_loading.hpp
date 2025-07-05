// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_LOADING_H__
#define __C_SHADER_LOADING_H__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma {
	class DLLCLIENT ShaderLoading : public ShaderGameWorldLightingPass {
	  public:
		ShaderLoading(prosper::IPrContext &context, const std::string &identifier);
	};
};

#endif
