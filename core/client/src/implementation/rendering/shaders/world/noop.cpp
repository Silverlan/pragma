// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_descriptor_set_group.hpp>

module pragma.client.rendering.shaders;

import :world_noop;

import pragma.client.client_state;
import pragma.client.engine;

extern ClientState *client;
extern CEngine *c_engine;

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
