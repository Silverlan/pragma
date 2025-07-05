// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_UNLIT_HPP__
#define __C_SHADER_UNLIT_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <mathutil/uvec.h>

namespace pragma {
	class DLLCLIENT ShaderUnlit : public ShaderGameWorldLightingPass {
	  public:
		ShaderUnlit(prosper::IPrContext &context, const std::string &identifier);
	};
};

#endif
