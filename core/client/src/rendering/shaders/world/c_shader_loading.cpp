/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_loading.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

using namespace pragma;

extern DLLCLIENT ClientState *client;

ShaderLoading::ShaderLoading(prosper::IPrContext &context,const std::string &identifier)
	: ShaderGameWorldLightingPass(context,identifier,"world/vs_textured","world/fs_loading")
{
	// SetBaseShader<ShaderTextured3DBase>();
}

bool ShaderLoading::BindMaterial(CMaterial&)
{
	auto *mat = client->GetMaterialManager().GetErrorMaterial();
	if(mat == nullptr)
		return false;
	return ShaderGameWorldLightingPass::BindMaterial(*static_cast<CMaterial*>(mat));
}
