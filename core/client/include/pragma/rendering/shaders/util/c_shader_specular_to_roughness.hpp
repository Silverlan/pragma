// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_SPECULAR_TO_ROUGHNESS_HPP__
#define __C_SHADER_SPECULAR_TO_ROUGHNESS_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {
	class Texture;
};
namespace pragma {
	class DLLCLIENT ShaderSpecularToRoughness : public prosper::ShaderBaseImageProcessing {
	  public:
		ShaderSpecularToRoughness(prosper::IPrContext &context, const std::string &identifier);
	};
};

#endif
