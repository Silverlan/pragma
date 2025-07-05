// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_loading.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <cmaterial_manager2.hpp>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT ClientState *client;

ShaderLoading::ShaderLoading(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "world/vs_textured", "world/fs_loading")
{
	// SetBaseShader<ShaderTextured3DBase>();
}
