// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_noop;

import :client_state;
import :engine;

using namespace pragma;

ShaderNoop::ShaderNoop(prosper::IPrContext &context, const std::string &identifier) : ShaderUnlit {context, identifier} {}

bool ShaderNoop::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, material::CMaterial &mat) const
{
	// Fail intentionally
	return false;
}

bool ShaderNoop::RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const
{
	// Fail intentionally
	return false;
}
