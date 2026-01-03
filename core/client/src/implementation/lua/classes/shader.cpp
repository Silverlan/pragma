// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.classes.shader;
import :rendering.shaders;

prosper::ShaderBindState *LuaShaderRecordTarget::GetBindState() const { return luabind::object_cast_nothrow<prosper::ShaderBindState *>(target, static_cast<prosper::ShaderBindState *>(nullptr)); }
prosper::util::PreparedCommandBuffer *LuaShaderRecordTarget::GetPcb() const { return luabind::object_cast_nothrow<prosper::util::PreparedCommandBuffer *>(target, static_cast<prosper::util::PreparedCommandBuffer *>(nullptr)); }
uint32_t Lua::Shader::AttachDescriptorSetInfo(lua::State *l, pragma::LuaShaderWrapperBase &shader, pragma::LuaDescriptorSetInfo &descSetInfo)
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
void Lua::Shader::AttachPushConstantRange(lua::State *l, pragma::LuaShaderWrapperBase &shader, uint32_t offset, uint32_t size, uint32_t shaderStages) { shader.GetShader().AttachPushConstantRange(offset, size, static_cast<prosper::ShaderStageFlags>(shaderStages)); }

void Lua::shader::push_shader(lua::State *l, prosper::Shader &shader)
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
void Lua::Shader::CreateDescriptorSetGroup(lua::State *l, prosper::Shader &shader, uint32_t setIdx)
{
	auto dsg = shader.CreateDescriptorSetGroup(setIdx);
	if(dsg == nullptr)
		return;
	Push(l, dsg);
}
void Lua::Shader::GetPipelineInfo(lua::State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx)
{
	auto *info = shader.GetPipelineInfo(pipelineIdx);
	if(info == nullptr)
		return;
	Push(l, std::ref(*info));
}
void Lua::Shader::GetEntrypointName(lua::State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx)
{
	auto *ep = shader.GetModuleStageEntryPoint(static_cast<prosper::ShaderStage>(shaderStage), pipelineIdx);
	if(ep == nullptr || ep->shader_module_ptr == nullptr)
		return;
	switch(shaderStage) {
	case pragma::math::to_integral(prosper::ShaderStageFlags::FragmentBit):
		PushString(l, ep->shader_module_ptr->GetFSEntrypointName());
		break;
	case pragma::math::to_integral(prosper::ShaderStageFlags::VertexBit):
		PushString(l, ep->shader_module_ptr->GetVSEntrypointName());
		break;
	case pragma::math::to_integral(prosper::ShaderStageFlags::GeometryBit):
		PushString(l, ep->shader_module_ptr->GetGSEntrypointName());
		break;
	case pragma::math::to_integral(prosper::ShaderStageFlags::TessellationControlBit):
		PushString(l, ep->shader_module_ptr->GetTCEntrypointName());
		break;
	case pragma::math::to_integral(prosper::ShaderStageFlags::TessellationEvaluationBit):
		PushString(l, ep->shader_module_ptr->GetTEEntrypointName());
		break;
	case pragma::math::to_integral(prosper::ShaderStageFlags::ComputeBit):
		PushString(l, ep->shader_module_ptr->GetCSEntrypointName());
		break;
	}
}
void Lua::Shader::GetGlslSourceCode(lua::State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx)
{
	auto *ep = shader.GetModuleStageEntryPoint(static_cast<prosper::ShaderStage>(shaderStage), pipelineIdx);
	if(ep == nullptr || ep->shader_module_ptr == nullptr)
		return;
	PushString(l, ep->shader_module_ptr->GetGLSLSourceCode());
}
void Lua::Shader::IsGraphicsShader(lua::State *l, prosper::Shader &shader) { PushBool(l, shader.IsGraphicsShader()); }
void Lua::Shader::IsComputeShader(lua::State *l, prosper::Shader &shader) { PushBool(l, shader.IsComputeShader()); }
void Lua::Shader::GetPipelineBindPoint(lua::State *l, prosper::Shader &shader) { PushInt(l, static_cast<int32_t>(shader.GetPipelineBindPoint())); }
void Lua::Shader::IsValid(lua::State *l, prosper::Shader &shader) { PushBool(l, shader.IsValid()); }
void Lua::Shader::GetIdentifier(lua::State *l, prosper::Shader &shader) { PushString(l, shader.GetIdentifier()); }
void Lua::Shader::GetSourceFilePath(lua::State *l, prosper::Shader &shader, uint32_t shaderStage)
{
	std::string sourceFilePath;
	auto r = shader.GetSourceFilePath(static_cast<prosper::ShaderStage>(shaderStage), sourceFilePath);
	if(r == false)
		return;
	PushString(l, sourceFilePath);
}
void Lua::Shader::GetSourceFilePaths(lua::State *l, prosper::Shader &shader)
{
	auto filePaths = shader.GetSourceFilePaths();
	auto t = CreateTable(l);
	auto idx = 1u;
	for(auto &fPath : filePaths) {
		PushInt(l, idx++);
		PushString(l, fPath);
		SetTableValue(l, t);
	}
}
void Lua::Shader::RecordPushConstants(lua::State *l, prosper::Shader &shader, prosper::util::PreparedCommandBuffer &pcb, ::udm::Type type, const Vulkan::PreparedCommandLuaArg &value, uint32_t offset)
{
	pcb.PushCommand(
	  [&shader, offset, type](const prosper::util::PreparedCommandBufferRecordState &recordState) mutable -> bool {
		  return ::udm::visit_ng(type, [&recordState, &shader, offset](auto tag) {
			  using T = typename decltype(tag)::type;
			  auto value = recordState.GetArgument<T>(0);
			  return shader.RecordPushConstants(*recordState.shaderBindState, sizeof(value), &value, offset);
		  });
	  },
	  pragma::util::make_vector<prosper::util::PreparedCommand::Argument>(Lua::Vulkan::make_pcb_arg(value, type)));
}
void Lua::Shader::RecordPushConstants(lua::State *l, prosper::Shader &shader, const LuaShaderRecordTarget &recordTarget, pragma::util::DataStream &ds, uint32_t offset)
{
	auto *bindState = recordTarget.GetBindState();
	if(bindState) {
		auto r = shader.RecordPushConstants(*bindState, ds->GetSize(), ds->GetData(), offset);
		PushBool(l, r);
		return;
	}
	recordTarget.GetPcb()->PushCommand([&shader, ds, offset](const prosper::util::PreparedCommandBufferRecordState &recordState) mutable -> bool { return shader.RecordPushConstants(*recordState.shaderBindState, ds->GetSize(), ds->GetData(), offset); });
}
static bool record_bind_descriptor_sets(prosper::Shader &shader, prosper::ShaderBindState &bindState, const std::vector<prosper::IDescriptorSet *> &descSets, uint32_t firstSet, const std::vector<uint32_t> &dynamicOffsets)
{
	return shader.RecordBindDescriptorSets(bindState, descSets, firstSet, dynamicOffsets);
}
void Lua::Shader::RecordBindDescriptorSet(lua::State *l, prosper::Shader &shader, prosper::util::PreparedCommandBuffer &pcb, Vulkan::DescriptorSet &ds, uint32_t firstSet, luabind::object oDynamicOffsets, std::optional<uint32_t> dynamicOffsetIndex)
{
	auto dynamicOffsets = dynamicOffsetIndex.has_value() ? Lua::table_to_vector<uint32_t>(l, oDynamicOffsets, *dynamicOffsetIndex) : std::vector<uint32_t> {};
	pcb.PushCommand(
	  [&shader, &ds, firstSet, dynamicOffsets = std::move(dynamicOffsets)](const prosper::util::PreparedCommandBufferRecordState &recordState) mutable -> bool { return shader.RecordBindDescriptorSet(*recordState.shaderBindState, *ds.GetDescriptorSet(), firstSet, dynamicOffsets); });
}
void Lua::Shader::RecordBindDescriptorSet(lua::State *l, prosper::Shader &shader, prosper::ShaderBindState &bindState, Vulkan::DescriptorSet &ds, uint32_t firstSet, luabind::object dynamicOffsets, std::optional<uint32_t> dynamicOffsetIndex)
{
	std::vector<uint32_t> vDynamicOffsets;
	if(dynamicOffsetIndex.has_value() && IsSet(l, *dynamicOffsetIndex)) {
		vDynamicOffsets = get_table_values<uint32_t>(l, *dynamicOffsetIndex, [](lua::State *l, int32_t idx) { return static_cast<uint32_t>(CheckInt(l, idx)); });
	}
	auto r = record_bind_descriptor_sets(shader, bindState, {ds.GetDescriptorSet()}, firstSet, vDynamicOffsets);
	PushBool(l, r);
}
void Lua::Shader::RecordBindDescriptorSets(lua::State *l, prosper::Shader &shader, prosper::ShaderBindState &bindState, luabind::object descSets, uint32_t firstSet, luabind::object dynamicOffsets)
{
	auto vDescSets = get_table_values<prosper::IDescriptorSet *>(l, 2u, [](lua::State *l, int32_t idx) { return Lua::Check<Vulkan::DescriptorSet>(l, idx).GetDescriptorSet(); });
	std::vector<uint32_t> vDynamicOffsets;
	if(IsSet(l, 4u)) {
		vDynamicOffsets = get_table_values<uint32_t>(l, 4u, [](lua::State *l, int32_t idx) { return static_cast<uint32_t>(CheckInt(l, idx)); });
	}
	auto r = record_bind_descriptor_sets(shader, bindState, vDescSets, firstSet, vDynamicOffsets);
	PushBool(l, r);
}

void Lua::Shader::SetStageSourceFilePath(lua::State *l, pragma::LuaShaderWrapperBase &shader, uint32_t shaderStage, const std::string &fpath) { shader.SetStageSourceFilePath(static_cast<prosper::ShaderStage>(shaderStage), fpath); }
void Lua::Shader::SetPipelineCount(lua::State *l, pragma::LuaShaderWrapperBase &shader, uint32_t pipelineCount) { shader.SetPipelineCount(pipelineCount); }
