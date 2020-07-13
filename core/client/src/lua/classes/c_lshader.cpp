/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

void Lua::BasePipelineCreateInfo::AttachDescriptorSetInfo(lua_State *l,prosper::BasePipelineCreateInfo &pipelineInfo,pragma::LuaDescriptorSetInfo &descSetInfo)
{
	auto *shader = pragma::LuaShaderBase::GetShader(pipelineInfo);
	if(shader == nullptr)
		return;
	prosper::DescriptorSetInfo shaderDescSetInfo {};
	shaderDescSetInfo.bindings.reserve(descSetInfo.bindings.size());
	auto bindingIdx = 0u;
	for(auto &lBinding : descSetInfo.bindings)
	{
		shaderDescSetInfo.bindings.push_back({});
		auto &binding = shaderDescSetInfo.bindings.back();
		binding.type = lBinding.type;
		binding.shaderStages = lBinding.shaderStages;
		binding.descriptorArraySize = lBinding.descriptorArraySize;
		binding.bindingIndex = (lBinding.bindingIndex != std::numeric_limits<uint32_t>::max()) ? lBinding.bindingIndex : bindingIdx;

		bindingIdx = binding.bindingIndex +1u;
	}
	shaderDescSetInfo.setIndex = descSetInfo.setIndex;
	shader->GetShader().AddDescriptorSetGroup(pipelineInfo,shaderDescSetInfo);
}
void Lua::BasePipelineCreateInfo::AttachPushConstantRange(lua_State *l,prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t offset,uint32_t size,uint32_t shaderStages)
{
	auto *shader = pragma::LuaShaderBase::GetShader(pipelineInfo);
	if(shader == nullptr)
		return;
	shader->GetShader().AttachPushConstantRange(pipelineInfo,offset,size,static_cast<prosper::ShaderStageFlags>(shaderStages));
}
void Lua::shader::push_shader(lua_State *l,prosper::Shader &shader)
{
	auto *luaShader = dynamic_cast<pragma::LuaShaderBase*>(&shader);
	if(luaShader != nullptr)
		luaShader->GetLuaObject().push(l);
	else
	{
		if(shader.IsGraphicsShader())
		{
			if(dynamic_cast<pragma::ShaderScene*>(&shader) != nullptr)
			{
				if(dynamic_cast<pragma::ShaderSceneLit*>(&shader) != nullptr)
				{
					if(dynamic_cast<pragma::ShaderEntity*>(&shader) != nullptr)
					{
						if(dynamic_cast<pragma::ShaderTextured3DBase*>(&shader) != nullptr)
							Lua::Push<pragma::ShaderTextured3DBase*>(l,static_cast<pragma::ShaderTextured3DBase*>(&shader));
						else
							Lua::Push<pragma::ShaderEntity*>(l,static_cast<pragma::ShaderEntity*>(&shader));
					}
					else
						Lua::Push<pragma::ShaderSceneLit*>(l,static_cast<pragma::ShaderSceneLit*>(&shader));
				}
				else
					Lua::Push<pragma::ShaderScene*>(l,static_cast<pragma::ShaderScene*>(&shader));
			}
			else if(dynamic_cast<pragma::ShaderComposeRMA*>(&shader))
				Lua::Push<pragma::ShaderComposeRMA*>(l,static_cast<pragma::ShaderComposeRMA*>(&shader));
			else
				Lua::Push<prosper::ShaderGraphics*>(l,static_cast<prosper::ShaderGraphics*>(&shader));
		}
		else if(shader.IsComputeShader())
			Lua::Push<prosper::ShaderCompute*>(l,static_cast<prosper::ShaderCompute*>(&shader));
		else
			Lua::Push<prosper::Shader*>(l,&shader);
	}
}
void Lua::Shader::CreateDescriptorSetGroup(lua_State *l,prosper::Shader &shader,uint32_t setIdx,uint32_t pipelineIdx)
{
	auto dsg = shader.CreateDescriptorSetGroup(setIdx,pipelineIdx);
	if(dsg == nullptr)
		return;
	Lua::Push(l,dsg);
}
void Lua::Shader::GetPipelineInfo(lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx)
{
	auto *info = shader.GetPipelineInfo(pipelineIdx);
	if(info == nullptr)
		return;
	Lua::Push(l,std::ref(*info));
}
void Lua::Shader::GetEntrypointName(lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx)
{
	auto *ep = shader.GetModuleStageEntryPoint(static_cast<prosper::ShaderStage>(shaderStage),pipelineIdx);
	if(ep == nullptr || ep->shader_module_ptr == nullptr)
		return;
	switch(shaderStage)
	{
		case umath::to_integral(prosper::ShaderStageFlags::FragmentBit):
			Lua::PushString(l,ep->shader_module_ptr->GetFSEntrypointName());
			break;
		case umath::to_integral(prosper::ShaderStageFlags::VertexBit):
			Lua::PushString(l,ep->shader_module_ptr->GetVSEntrypointName());
			break;
		case umath::to_integral(prosper::ShaderStageFlags::GeometryBit):
			Lua::PushString(l,ep->shader_module_ptr->GetGSEntrypointName());
			break;
		case umath::to_integral(prosper::ShaderStageFlags::TessellationControlBit):
			Lua::PushString(l,ep->shader_module_ptr->GetTCEntrypointName());
			break;
		case umath::to_integral(prosper::ShaderStageFlags::TessellationEvaluationBit):
			Lua::PushString(l,ep->shader_module_ptr->GetTEEntrypointName());
			break;
		case umath::to_integral(prosper::ShaderStageFlags::ComputeBit):
			Lua::PushString(l,ep->shader_module_ptr->GetCSEntrypointName());
			break;
	}
}
void Lua::Shader::GetGlslSourceCode(lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx)
{
	auto *ep = shader.GetModuleStageEntryPoint(static_cast<prosper::ShaderStage>(shaderStage),pipelineIdx);
	if(ep == nullptr || ep->shader_module_ptr == nullptr)
		return;
	Lua::PushString(l,ep->shader_module_ptr->GetGLSLSourceCode());
}
void Lua::Shader::IsGraphicsShader(lua_State *l,prosper::Shader &shader) {Lua::PushBool(l,shader.IsGraphicsShader());}
void Lua::Shader::IsComputeShader(lua_State *l,prosper::Shader &shader) {Lua::PushBool(l,shader.IsComputeShader());}
void Lua::Shader::GetPipelineBindPoint(lua_State *l,prosper::Shader &shader) {Lua::PushInt(l,static_cast<int32_t>(shader.GetPipelineBindPoint()));}
void Lua::Shader::IsValid(lua_State *l,prosper::Shader &shader) {Lua::PushBool(l,shader.IsValid());}
void Lua::Shader::GetIdentifier(lua_State *l,prosper::Shader &shader) {Lua::PushString(l,shader.GetIdentifier());}
void Lua::Shader::GetSourceFilePath(lua_State *l,prosper::Shader &shader,uint32_t shaderStage)
{
	std::string sourceFilePath;
	auto r = shader.GetSourceFilePath(static_cast<prosper::ShaderStage>(shaderStage),sourceFilePath);
	if(r == false)
		return;
	Lua::PushString(l,sourceFilePath);
}
void Lua::Shader::GetSourceFilePaths(lua_State *l,prosper::Shader &shader)
{
	auto filePaths = shader.GetSourceFilePaths();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &fPath : filePaths)
	{
		Lua::PushInt(l,idx++);
		Lua::PushString(l,fPath);
		Lua::SetTableValue(l,t);
	}
}
void Lua::Shader::RecordPushConstants(lua_State *l,prosper::Shader &shader,::DataStream &ds,uint32_t offset)
{
	auto r = shader.RecordPushConstants(ds->GetSize(),ds->GetData(),offset);
	Lua::PushBool(l,r);
}
static bool record_bind_descriptor_sets(prosper::Shader &shader,const std::vector<prosper::IDescriptorSet*> &descSets,uint32_t firstSet,const std::vector<uint32_t> &dynamicOffsets)
{
	return shader.RecordBindDescriptorSets(descSets,firstSet,dynamicOffsets);
}
void Lua::Shader::RecordBindDescriptorSet(lua_State *l,prosper::Shader &shader,Lua::Vulkan::DescriptorSet &ds,uint32_t firstSet,luabind::object dynamicOffsets)
{
	std::vector<uint32_t> vDynamicOffsets;
	if(Lua::IsSet(l,4u))
	{
		vDynamicOffsets = get_table_values<uint32_t>(l,4u,[](lua_State *l,int32_t idx) {
			return static_cast<uint32_t>(Lua::CheckInt(l,idx));
		});
	}
	auto r = record_bind_descriptor_sets(shader,{ds.GetDescriptorSet()},firstSet,vDynamicOffsets);
	Lua::PushBool(l,r);
}
void Lua::Shader::RecordBindDescriptorSets(lua_State *l,prosper::Shader &shader,luabind::object descSets,uint32_t firstSet,luabind::object dynamicOffsets)
{
	auto vDescSets = get_table_values<prosper::IDescriptorSet*>(l,2u,[](lua_State *l,int32_t idx) {
		return Lua::Check<Lua::Vulkan::DescriptorSet>(l,idx).GetDescriptorSet();
	});
	std::vector<uint32_t> vDynamicOffsets;
	if(Lua::IsSet(l,4u))
	{
		vDynamicOffsets = get_table_values<uint32_t>(l,4u,[](lua_State *l,int32_t idx) {
			return static_cast<uint32_t>(Lua::CheckInt(l,idx));
		});
	}
	auto r = record_bind_descriptor_sets(shader,vDescSets,firstSet,vDynamicOffsets);
	Lua::PushBool(l,r);
}

void Lua::Shader::SetStageSourceFilePath(lua_State *l,pragma::LuaShaderBase &shader,uint32_t shaderStage,const std::string &fpath)
{
	shader.GetShader().SetStageSourceFilePath(static_cast<prosper::ShaderStage>(shaderStage),fpath);
}
void Lua::Shader::SetPipelineCount(lua_State *l,pragma::LuaShaderBase &shader,uint32_t pipelineCount)
{
	shader.SetPipelineCount(pipelineCount);
}
void Lua::Shader::GetCurrentCommandBuffer(lua_State *l,pragma::LuaShaderBase &shader)
{
	auto wpDrawCmd = shader.GetCurrentCommandBuffer();
	if(wpDrawCmd == nullptr)
		return;
	Lua::Push(l,std::static_pointer_cast<prosper::ICommandBuffer>(wpDrawCmd));
}
