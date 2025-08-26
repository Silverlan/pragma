// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include <shader/prosper_pipeline_create_info.hpp>

module pragma.client.rendering.shaders;

import :extract_diffuse_ambinet_occlusion;

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

ShaderExtractDiffuseAmbientOcclusion::ShaderExtractDiffuseAmbientOcclusion(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/util/extract_diffuse_ambient_occlusion"} {}
