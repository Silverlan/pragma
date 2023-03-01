/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/util/c_shader_extract_diffuse_ambient_occlusion.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

ShaderExtractDiffuseAmbientOcclusion::ShaderExtractDiffuseAmbientOcclusion(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "util/fs_extract_diffuse_ambient_occlusion.gls"} {}
