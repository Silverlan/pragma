/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/util/c_shader_specular_to_roughness.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;


ShaderSpecularToRoughness::ShaderSpecularToRoughness(prosper::IPrContext &context,const std::string &identifier)
	: ShaderBaseImageProcessing{context,identifier,"util/fs_specular_to_roughness.gls"}
{}

