// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :scripting.lua.classes.shader;
void Lua::ComputePipelineCreateInfo::AddSpecializationConstant(lua::State *l, prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t constantId, ::util::DataStream &ds) { Lua::PushBool(l, pipelineInfo.AddSpecializationConstant(constantId, ds->GetSize(), ds->GetData())); }

void Lua::Shader::Compute::RecordDispatch(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState, uint32_t x, uint32_t y, uint32_t z) { Lua::PushBool(l, shader.RecordDispatch(bindState, x, y, z)); }
void Lua::Shader::Compute::RecordBeginCompute(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState, uint32_t pipelineIdx) { Lua::PushBool(l, shader.RecordBeginCompute(bindState, pipelineIdx)); }
void Lua::Shader::Compute::RecordCompute(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState)
{
	shader.RecordCompute(bindState);
	Lua::PushBool(l, true);
}
void Lua::Shader::Compute::RecordEndCompute(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState)
{
	shader.RecordEndCompute(bindState);
	Lua::PushBool(l, true);
}
