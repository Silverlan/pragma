// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_loading;

using namespace pragma;

ShaderLoading::ShaderLoading(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "world/vs_textured", "world/fs_loading")
{
	// SetBaseShader<ShaderTextured3DBase>();
}
