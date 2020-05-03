/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_EXTRACT_DIFFUSE_AMBIENT_OCCLUSION_HPP__
#define __C_SHADER_EXTRACT_DIFFUSE_AMBIENT_OCCLUSION_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {class Texture;};
namespace pragma
{
	class DLLCLIENT ShaderExtractDiffuseAmbientOcclusion
		: public prosper::ShaderBaseImageProcessing
	{
	public:
		ShaderExtractDiffuseAmbientOcclusion(prosper::IPrContext &context,const std::string &identifier);
	};
};

#endif
