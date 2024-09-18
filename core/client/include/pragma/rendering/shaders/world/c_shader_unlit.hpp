/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
