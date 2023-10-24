/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_noop.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

ShaderNoop::ShaderNoop(prosper::IPrContext &context, const std::string &identifier) : ShaderUnlit {context, identifier} {}

bool ShaderNoop::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const
{
	// Fail intentionally
	return false;
}

bool ShaderNoop::RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const
{
	// Fail intentionally
	return false;
}
