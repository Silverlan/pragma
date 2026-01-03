// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.classes.shader;
void Lua::ComputePipelineCreateInfo::AddSpecializationConstant(lua::State *l, prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t constantId, pragma::util::DataStream &ds) { PushBool(l, pipelineInfo.AddSpecializationConstant(constantId, ds->GetSize(), ds->GetData())); }

void Lua::Shader::Compute::RecordDispatch(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState, uint32_t x, uint32_t y, uint32_t z) { PushBool(l, shader.RecordDispatch(bindState, x, y, z)); }
void Lua::Shader::Compute::RecordBeginCompute(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState, uint32_t pipelineIdx) { PushBool(l, shader.RecordBeginCompute(bindState, pipelineIdx)); }
void Lua::Shader::Compute::RecordCompute(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState)
{
	shader.RecordCompute(bindState);
	PushBool(l, true);
}
void Lua::Shader::Compute::RecordEndCompute(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState)
{
	shader.RecordEndCompute(bindState);
	PushBool(l, true);
}
