// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.extract_diffuse_ambinet_occlusion;

import :engine;

using namespace pragma;

ShaderExtractDiffuseAmbientOcclusion::ShaderExtractDiffuseAmbientOcclusion(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/util/extract_diffuse_ambient_occlusion"} {}
