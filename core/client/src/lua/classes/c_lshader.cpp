/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"
#include "pragma/rendering/shaders/image/c_shader_merge_images.hpp"
#include "pragma/rendering/shaders/image/c_shader_merge_2d_image_into_equirectangular.hpp"
#include "pragma/lua/converters/shader_converter_t.hpp"
#include <pragma/lua/util.hpp>
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_flip_image.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_prepared_command_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

prosper::ShaderBindState *LuaShaderRecordTarget::GetBindState() const { return luabind::object_cast_nothrow<prosper::ShaderBindState *>(target, static_cast<prosper::ShaderBindState *>(nullptr)); }
prosper::util::PreparedCommandBuffer *LuaShaderRecordTarget::GetPcb() const { return luabind::object_cast_nothrow<prosper::util::PreparedCommandBuffer *>(target, static_cast<prosper::util::PreparedCommandBuffer *>(nullptr)); }
uint32_t Lua::Shader::AttachDescriptorSetInfo(lua_State *l, pragma::LuaShaderWrapperBase &shader, pragma::LuaDescriptorSetInfo &descSetInfo)
{
	prosper::DescriptorSetInfo shaderDescSetInfo {pragma::register_global_string(descSetInfo.name), {}};
	shaderDescSetInfo.bindings.reserve(descSetInfo.bindings.size());
	auto bindingIdx = 0u;
	for(auto &lBinding : descSetInfo.bindings) {
		shaderDescSetInfo.bindings.push_back({});
		auto &binding = shaderDescSetInfo.bindings.back();
		binding.name = pragma::register_global_string(lBinding.name);
		binding.type = lBinding.type;
		binding.shaderStages = lBinding.shaderStages;
		binding.descriptorArraySize = lBinding.descriptorArraySize;
		binding.bindingIndex = (lBinding.bindingIndex != std::numeric_limits<uint32_t>::max()) ? lBinding.bindingIndex : bindingIdx;

		bindingIdx = binding.bindingIndex + 1u;
	}
	shaderDescSetInfo.setIndex = descSetInfo.setIndex;
	return shader.GetShader().AddDescriptorSetGroup(shaderDescSetInfo);
}
void Lua::Shader::AttachPushConstantRange(lua_State *l, pragma::LuaShaderWrapperBase &shader, uint32_t offset, uint32_t size, uint32_t shaderStages) { shader.GetShader().AttachPushConstantRange(offset, size, static_cast<prosper::ShaderStageFlags>(shaderStages)); }

void Lua::shader::push_shader(lua_State *l, prosper::Shader &shader)
{
	auto *luaShader = dynamic_cast<pragma::LuaShaderWrapperBase *>(&shader);
	if(luaShader != nullptr)
		luaShader->GetLuaObject().push(l);
	else {
		if(shader.IsGraphicsShader()) {
			if(dynamic_cast<pragma::ShaderScene *>(&shader) != nullptr) {
				if(dynamic_cast<pragma::ShaderSceneLit *>(&shader) != nullptr) {
					if(dynamic_cast<pragma::ShaderEntity *>(&shader) != nullptr) {
						if(dynamic_cast<pragma::ShaderGameWorldLightingPass *>(&shader) != nullptr)
							Lua::PushRaw<pragma::ShaderGameWorldLightingPass *>(l, static_cast<pragma::ShaderGameWorldLightingPass *>(&shader));
						else
							Lua::PushRaw<pragma::ShaderEntity *>(l, static_cast<pragma::ShaderEntity *>(&shader));
					}
					else
						Lua::PushRaw<pragma::ShaderSceneLit *>(l, static_cast<pragma::ShaderSceneLit *>(&shader));
				}
				else
					Lua::PushRaw<pragma::ShaderScene *>(l, static_cast<pragma::ShaderScene *>(&shader));
			}
			else if(dynamic_cast<pragma::ShaderComposeRMA *>(&shader))
				Lua::PushRaw<pragma::ShaderComposeRMA *>(l, static_cast<pragma::ShaderComposeRMA *>(&shader));
			else if(dynamic_cast<prosper::ShaderFlipImage *>(&shader))
				Lua::PushRaw<prosper::ShaderFlipImage *>(l, static_cast<prosper::ShaderFlipImage *>(&shader));
			else if(dynamic_cast<pragma::ShaderMergeImages *>(&shader))
				Lua::PushRaw<pragma::ShaderMergeImages *>(l, static_cast<pragma::ShaderMergeImages *>(&shader));
			else if(dynamic_cast<pragma::ShaderMerge2dImageIntoEquirectangular *>(&shader))
				Lua::PushRaw<pragma::ShaderMerge2dImageIntoEquirectangular *>(l, static_cast<pragma::ShaderMerge2dImageIntoEquirectangular *>(&shader));
			else
				Lua::PushRaw<prosper::ShaderGraphics *>(l, static_cast<prosper::ShaderGraphics *>(&shader));
		}
		else if(shader.IsComputeShader())
			Lua::PushRaw<prosper::ShaderCompute *>(l, static_cast<prosper::ShaderCompute *>(&shader));
		else
			Lua::PushRaw<prosper::Shader *>(l, &shader);
	}
}
void Lua::Shader::CreateDescriptorSetGroup(lua_State *l, prosper::Shader &shader, uint32_t setIdx)
{
	auto dsg = shader.CreateDescriptorSetGroup(setIdx);
	if(dsg == nullptr)
		return;
	Lua::Push(l, dsg);
}
void Lua::Shader::GetPipelineInfo(lua_State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx)
{
	auto *info = shader.GetPipelineInfo(pipelineIdx);
	if(info == nullptr)
		return;
	Lua::Push(l, std::ref(*info));
}
void Lua::Shader::GetEntrypointName(lua_State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx)
{
	auto *ep = shader.GetModuleStageEntryPoint(static_cast<prosper::ShaderStage>(shaderStage), pipelineIdx);
	if(ep == nullptr || ep->shader_module_ptr == nullptr)
		return;
	switch(shaderStage) {
	case umath::to_integral(prosper::ShaderStageFlags::FragmentBit):
		Lua::PushString(l, ep->shader_module_ptr->GetFSEntrypointName());
		break;
	case umath::to_integral(prosper::ShaderStageFlags::VertexBit):
		Lua::PushString(l, ep->shader_module_ptr->GetVSEntrypointName());
		break;
	case umath::to_integral(prosper::ShaderStageFlags::GeometryBit):
		Lua::PushString(l, ep->shader_module_ptr->GetGSEntrypointName());
		break;
	case umath::to_integral(prosper::ShaderStageFlags::TessellationControlBit):
		Lua::PushString(l, ep->shader_module_ptr->GetTCEntrypointName());
		break;
	case umath::to_integral(prosper::ShaderStageFlags::TessellationEvaluationBit):
		Lua::PushString(l, ep->shader_module_ptr->GetTEEntrypointName());
		break;
	case umath::to_integral(prosper::ShaderStageFlags::ComputeBit):
		Lua::PushString(l, ep->shader_module_ptr->GetCSEntrypointName());
		break;
	}
}
void Lua::Shader::GetGlslSourceCode(lua_State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx)
{
	auto *ep = shader.GetModuleStageEntryPoint(static_cast<prosper::ShaderStage>(shaderStage), pipelineIdx);
	if(ep == nullptr || ep->shader_module_ptr == nullptr)
		return;
	Lua::PushString(l, ep->shader_module_ptr->GetGLSLSourceCode());
}
void Lua::Shader::IsGraphicsShader(lua_State *l, prosper::Shader &shader) { Lua::PushBool(l, shader.IsGraphicsShader()); }
void Lua::Shader::IsComputeShader(lua_State *l, prosper::Shader &shader) { Lua::PushBool(l, shader.IsComputeShader()); }
void Lua::Shader::GetPipelineBindPoint(lua_State *l, prosper::Shader &shader) { Lua::PushInt(l, static_cast<int32_t>(shader.GetPipelineBindPoint())); }
void Lua::Shader::IsValid(lua_State *l, prosper::Shader &shader) { Lua::PushBool(l, shader.IsValid()); }
void Lua::Shader::GetIdentifier(lua_State *l, prosper::Shader &shader) { Lua::PushString(l, shader.GetIdentifier()); }
void Lua::Shader::GetSourceFilePath(lua_State *l, prosper::Shader &shader, uint32_t shaderStage)
{
	std::string sourceFilePath;
	auto r = shader.GetSourceFilePath(static_cast<prosper::ShaderStage>(shaderStage), sourceFilePath);
	if(r == false)
		return;
	Lua::PushString(l, sourceFilePath);
}
void Lua::Shader::GetSourceFilePaths(lua_State *l, prosper::Shader &shader)
{
	auto filePaths = shader.GetSourceFilePaths();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &fPath : filePaths) {
		Lua::PushInt(l, idx++);
		Lua::PushString(l, fPath);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Shader::RecordPushConstants(lua_State *l, prosper::Shader &shader, prosper::util::PreparedCommandBuffer &pcb, udm::Type type, const Lua::Vulkan::PreparedCommandLuaArg &value, uint32_t offset)
{
	pcb.PushCommand(
	  [&shader, offset, type](const prosper::util::PreparedCommandBufferRecordState &recordState) mutable -> bool {
		  return udm::visit_ng(type, [&recordState, &shader, offset](auto tag) {
			  using T = typename decltype(tag)::type;
			  auto value = recordState.GetArgument<T>(0);
			  return shader.RecordPushConstants(*recordState.shaderBindState, sizeof(value), &value, offset);
		  });
	  },
	  util::make_vector<prosper::util::PreparedCommand::Argument>(Lua::Vulkan::make_pcb_arg(value, type)));
}
void Lua::Shader::RecordPushConstants(lua_State *l, prosper::Shader &shader, const LuaShaderRecordTarget &recordTarget, ::DataStream &ds, uint32_t offset)
{
	auto *bindState = recordTarget.GetBindState();
	if(bindState) {
		auto r = shader.RecordPushConstants(*bindState, ds->GetSize(), ds->GetData(), offset);
		Lua::PushBool(l, r);
		return;
	}
	recordTarget.GetPcb()->PushCommand([&shader, ds, offset](const prosper::util::PreparedCommandBufferRecordState &recordState) mutable -> bool { return shader.RecordPushConstants(*recordState.shaderBindState, ds->GetSize(), ds->GetData(), offset); });
}
static bool record_bind_descriptor_sets(prosper::Shader &shader, prosper::ShaderBindState &bindState, const std::vector<prosper::IDescriptorSet *> &descSets, uint32_t firstSet, const std::vector<uint32_t> &dynamicOffsets)
{
	return shader.RecordBindDescriptorSets(bindState, descSets, firstSet, dynamicOffsets);
}
void Lua::Shader::RecordBindDescriptorSet(lua_State *l, prosper::Shader &shader, prosper::util::PreparedCommandBuffer &pcb, Lua::Vulkan::DescriptorSet &ds, uint32_t firstSet, luabind::object oDynamicOffsets, std::optional<uint32_t> dynamicOffsetIndex)
{
	auto dynamicOffsets = dynamicOffsetIndex.has_value() ? Lua::table_to_vector<uint32_t>(l, oDynamicOffsets, *dynamicOffsetIndex) : std::vector<uint32_t> {};
	pcb.PushCommand(
	  [&shader, &ds, firstSet, dynamicOffsets = std::move(dynamicOffsets)](const prosper::util::PreparedCommandBufferRecordState &recordState) mutable -> bool { return shader.RecordBindDescriptorSet(*recordState.shaderBindState, *ds.GetDescriptorSet(), firstSet, dynamicOffsets); });
}
void Lua::Shader::RecordBindDescriptorSet(lua_State *l, prosper::Shader &shader, prosper::ShaderBindState &bindState, Lua::Vulkan::DescriptorSet &ds, uint32_t firstSet, luabind::object dynamicOffsets, std::optional<uint32_t> dynamicOffsetIndex)
{
	std::vector<uint32_t> vDynamicOffsets;
	if(dynamicOffsetIndex.has_value() && Lua::IsSet(l, *dynamicOffsetIndex)) {
		vDynamicOffsets = get_table_values<uint32_t>(l, *dynamicOffsetIndex, [](lua_State *l, int32_t idx) { return static_cast<uint32_t>(Lua::CheckInt(l, idx)); });
	}
	auto r = record_bind_descriptor_sets(shader, bindState, {ds.GetDescriptorSet()}, firstSet, vDynamicOffsets);
	Lua::PushBool(l, r);
}
void Lua::Shader::RecordBindDescriptorSets(lua_State *l, prosper::Shader &shader, prosper::ShaderBindState &bindState, luabind::object descSets, uint32_t firstSet, luabind::object dynamicOffsets)
{
	auto vDescSets = get_table_values<prosper::IDescriptorSet *>(l, 2u, [](lua_State *l, int32_t idx) { return Lua::Check<Lua::Vulkan::DescriptorSet>(l, idx).GetDescriptorSet(); });
	std::vector<uint32_t> vDynamicOffsets;
	if(Lua::IsSet(l, 4u)) {
		vDynamicOffsets = get_table_values<uint32_t>(l, 4u, [](lua_State *l, int32_t idx) { return static_cast<uint32_t>(Lua::CheckInt(l, idx)); });
	}
	auto r = record_bind_descriptor_sets(shader, bindState, vDescSets, firstSet, vDynamicOffsets);
	Lua::PushBool(l, r);
}

void Lua::Shader::SetStageSourceFilePath(lua_State *l, pragma::LuaShaderWrapperBase &shader, uint32_t shaderStage, const std::string &fpath) { shader.SetStageSourceFilePath(static_cast<prosper::ShaderStage>(shaderStage), fpath); }
void Lua::Shader::SetPipelineCount(lua_State *l, pragma::LuaShaderWrapperBase &shader, uint32_t pipelineCount) { shader.SetPipelineCount(pipelineCount); }
