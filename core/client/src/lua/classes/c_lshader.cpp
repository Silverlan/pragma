#include "stdafx_client.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <misc/compute_pipeline_create_info.h>

void Lua::BasePipelineCreateInfo::AttachDescriptorSetInfo(lua_State *l,Anvil::BasePipelineCreateInfo &pipelineInfo,pragma::LuaDescriptorSetInfo &descSetInfo)
{
	auto *shader = pragma::LuaShaderBase::GetShader(pipelineInfo);
	if(shader == nullptr)
		return;
	prosper::Shader::DescriptorSetInfo shaderDescSetInfo {};
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
void Lua::BasePipelineCreateInfo::AttachPushConstantRange(lua_State *l,Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t offset,uint32_t size,uint32_t shaderStages)
{
	Lua::PushBool(l,pipelineInfo.attach_push_constant_range(offset,size,static_cast<Anvil::ShaderStageFlagBits>(shaderStages)));
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
	auto *ep = shader.GetModuleStageEntryPoint(static_cast<Anvil::ShaderStage>(shaderStage),pipelineIdx);
	if(ep == nullptr || ep->shader_module_ptr == nullptr)
		return;
	switch(shaderStage)
	{
		case umath::to_integral(Anvil::ShaderStageFlagBits::FRAGMENT_BIT):
			Lua::PushString(l,ep->shader_module_ptr->get_fs_entrypoint_name());
			break;
		case umath::to_integral(Anvil::ShaderStageFlagBits::VERTEX_BIT):
			Lua::PushString(l,ep->shader_module_ptr->get_vs_entrypoint_name());
			break;
		case umath::to_integral(Anvil::ShaderStageFlagBits::GEOMETRY_BIT):
			Lua::PushString(l,ep->shader_module_ptr->get_gs_entrypoint_name());
			break;
		case umath::to_integral(vk::ShaderStageFlagBits::eTessellationControl):
			Lua::PushString(l,ep->shader_module_ptr->get_tc_entrypoint_name());
			break;
		case umath::to_integral(vk::ShaderStageFlagBits::eTessellationEvaluation):
			Lua::PushString(l,ep->shader_module_ptr->get_te_entrypoint_name());
			break;
		case umath::to_integral(Anvil::ShaderStageFlagBits::COMPUTE_BIT):
			Lua::PushString(l,ep->shader_module_ptr->get_cs_entrypoint_name());
			break;
	}
}
void Lua::Shader::GetGlslSourceCode(lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx)
{
	auto *ep = shader.GetModuleStageEntryPoint(static_cast<Anvil::ShaderStage>(shaderStage),pipelineIdx);
	if(ep == nullptr || ep->shader_module_ptr == nullptr)
		return;
	Lua::PushString(l,ep->shader_module_ptr->get_glsl_source_code());
}
void Lua::Shader::GetSpirvBlob(lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx)
{
	auto *ep = shader.GetModuleStageEntryPoint(static_cast<Anvil::ShaderStage>(shaderStage),pipelineIdx);
	if(ep == nullptr || ep->shader_module_ptr == nullptr)
		return;
	auto &spirvBlob = ep->shader_module_ptr->get_spirv_blob();
	auto sz = spirvBlob.size() *sizeof(spirvBlob.front());
	auto ds = DataStream(sz);
	ds->Write(reinterpret_cast<const uint8_t*>(spirvBlob.data()),sz);
	Lua::Push(l,ds);
}
void Lua::Shader::GetStatistics(lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx)
{
	vk::ShaderStatisticsInfoAMD stats;
	if(shader.GetShaderStatistics(stats,static_cast<Anvil::ShaderStage>(shaderStage),pipelineIdx) == false)
		return;
	Lua::Push(l,stats);
}
void Lua::Shader::IsGraphicsShader(lua_State *l,prosper::Shader &shader) {Lua::PushBool(l,shader.IsGraphicsShader());}
void Lua::Shader::IsComputeShader(lua_State *l,prosper::Shader &shader) {Lua::PushBool(l,shader.IsComputeShader());}
void Lua::Shader::GetPipelineBindPoint(lua_State *l,prosper::Shader &shader) {Lua::PushInt(l,static_cast<int32_t>(shader.GetPipelineBindPoint()));}
void Lua::Shader::IsValid(lua_State *l,prosper::Shader &shader) {Lua::PushBool(l,shader.IsValid());}
void Lua::Shader::GetIdentifier(lua_State *l,prosper::Shader &shader) {Lua::PushString(l,shader.GetIdentifier());}
void Lua::Shader::GetSourceFilePath(lua_State *l,prosper::Shader &shader,uint32_t shaderStage)
{
	std::string sourceFilePath;
	auto r = shader.GetSourceFilePath(static_cast<Anvil::ShaderStage>(shaderStage),sourceFilePath);
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
static bool record_bind_descriptor_sets(prosper::Shader &shader,const std::vector<Anvil::DescriptorSet*> &descSets,uint32_t firstSet,const std::vector<uint32_t> &dynamicOffsets)
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
	auto r = record_bind_descriptor_sets(shader,{ds->get_descriptor_set(0u)},firstSet,vDynamicOffsets);
	Lua::PushBool(l,r);
}
void Lua::Shader::RecordBindDescriptorSets(lua_State *l,prosper::Shader &shader,luabind::object descSets,uint32_t firstSet,luabind::object dynamicOffsets)
{
	auto vDescSets = get_table_values<Anvil::DescriptorSet*>(l,2u,[](lua_State *l,int32_t idx) {
		return Lua::Check<Lua::Vulkan::DescriptorSet>(l,idx)->get_descriptor_set(0u);
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
	shader.GetShader().SetStageSourceFilePath(static_cast<Anvil::ShaderStage>(shaderStage),fpath);
}
void Lua::Shader::SetPipelineCount(lua_State *l,pragma::LuaShaderBase &shader,uint32_t pipelineCount)
{
	shader.SetPipelineCount(pipelineCount);
}
void Lua::Shader::GetCurrentCommandBuffer(lua_State *l,pragma::LuaShaderBase &shader)
{
	auto wpDrawCmd = shader.GetCurrentCommandBuffer();
	if(wpDrawCmd)
		return;
	Lua::Push(l,std::static_pointer_cast<prosper::CommandBuffer>(wpDrawCmd));
}

 // prosper TODO
#if 0
#include "pragma/lua/classes/c_lshader.h"
#include "luasystem.h"
#include "pragma/rendering/shaders/c_shaderlua.h"
#include "pragma/rendering/shaders/debug/c_shader_debug.h"
#include "pragma/lua/classes/c_ldef_camera.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/classes/ldatastream.h>
#include <pragma/lua/classes/ldef_entity.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

template<class TShader>
	bool push_shader(lua_State *l,const ShaderHandle &hShader)
{
	auto *shader = dynamic_cast<TShader*>(hShader.get());
	if(shader != nullptr)
	{
		Lua::Push<TShader*>(l,shader);
		return true;
	}
	return false;
}

bool Lua::shader::push(lua_State *l,::Shader::Base &shader)
{
	auto hShader = shader.GetHandle();
	auto *luaShader = dynamic_cast<::Shader::Lua::LuaBase*>(&shader);
	if(luaShader != nullptr)
	{
		luaShader->GetLuaObject().push(l);
		return true;
	}
	else if(push_shader<::Shader::Debug>(l,hShader) == false && push_shader<::Shader::TexturedBase3D>(l,hShader) == false && push_shader<::Shader::Base3D>(l,hShader) == false)
	{
		Lua::Push<::Shader::Base*>(l,hShader.get());
		return true;
	}
	return true;
}
int Lua::shader::get(lua_State *l)
{
	auto *name = Lua::CheckString(l,1);

	//auto &manager = c_game->GetLuaShaderManager();
	auto hShader = ShaderSystem::get_shader(name);
	if(hShader.IsValid() == false || push(l,*hShader.get()) ==  false)
		return 0;
	return 1;
}

int Lua::shader::register_shader(lua_State *l)
{
	auto *className = Lua::CheckString(l,1);
	luaL_checkuserdata(l,2);
	auto o = luabind::object(luabind::from_stack(l,2));
	if(o)
	{
		auto &manager = c_game->GetLuaShaderManager();
		manager.RegisterShader(className,o);
	}
	return 0;
	/*std::string className = Lua::CheckString(l,1);
	luaL_checkuserdata(l,2);
	ustring::to_lower(className);

	LuaBaseShader *shader = nullptr;
	auto *sh = ShaderSystem::GetShader(className);
	if(sh != nullptr && dynamic_cast<LuaBaseShader*>(sh) == nullptr)
	{
		Con::cwar<<"WARNING: Unable to overwrite existing shader '"<<className<<"'!"<<Con::endl;
		return 0;
	}
	else
		shader = static_cast<LuaBaseShader*>(sh);

	auto o = luabind::object(luabind::from_stack(l,2));
	if(o)
	{
		luabind::object r;
		try
		{
			r = o();
		}
		catch(luabind::error&)
		{
			Lua::InvokeErrorHandle(l);
			return 0;
		}
		if(!r)
		{
			Con::csv<<"WARNING: Unable to create lua shader '"<<className<<"'!"<<Con::endl;
			return 0;
		}
		if(!luabind::object_cast_nothrow<LuaBaseShader*>(r))
		{
			Con::csv<<"WARNING: Unable to create lua shader '"<<className<<"': Lua class is not derived from valid shader base!"<<Con::endl;
			return 0;
		}
		if(shader == nullptr)
		{
			shader = new LuaBaseShader(className,r);
			ShaderSystem::RegisterShader(className,shader);
		}
		else
			shader->Reset(r);
		if(shader != nullptr)
		{
			auto &materials = CMaterialSystem::GetMaterials();
			for(auto it=materials.begin();it!=materials.end();++it)
			{
				auto &hMaterial = it->second;
				if(hMaterial.IsValid())
				{
					auto *mat = hMaterial.get();
					if(mat->GetShader() == className)
						shader->InitializeMaterial(mat);
				}
			}
		}
	}*/ // Vulkan TODO
	return 0;
}

void Lua::Shader::GenerateDescriptorSet(lua_State *l,::Shader::Base &shader,uint32_t layoutId)
{
	::Vulkan::DescriptorSet descSet = nullptr;
	if(shader.GenerateDescriptorSet(layoutId,descSet) == false)
		return;
	Lua::Push<Vulkan::DescriptorSetHandleOwner>(l,Vulkan::DescriptorSetHandleOwner(descSet->GetHandle(),descSet));
}
void Lua::Shader::GenerateSwapDescriptorBuffer(lua_State *l,::Shader::Base &shader,uint32_t layoutId,uint32_t usage,uint32_t size)
{
	::Vulkan::SwapDescriptorBuffer swapBuffer = nullptr;
	if(shader.GenerateSwapDescriptorBuffer(layoutId,static_cast<vk::BufferUsageFlagBits>(usage),size,swapBuffer) == false)
		return;
	Lua::Push<::Vulkan::SwapDescriptorBuffer>(l,swapBuffer);
}
static void push_constants(lua_State *l,::Shader::Base &shader,uint32_t shaderStages,uint32_t offset,::DataStream &ds,int32_t dsArg)
{
	Lua::CheckDataStream(l,dsArg);
	auto &context = c_engine->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	drawCmd->PushConstants(shader.GetPipeline()->GetPipelineLayout(),static_cast<vk::ShaderStageFlagBits>(shaderStages),offset /sizeof(uint32_t),ds->GetSize() /sizeof(uint32_t),ds->GetData());
}
void Lua::Shader::PushConstants(lua_State *l,::Shader::Base &shader,uint32_t shaderStages,uint32_t offset,::DataStream &ds) {push_constants(l,shader,shaderStages,offset,ds,4);}
void Lua::Shader::PushConstants(lua_State *l,::Shader::Base &shader,uint32_t shaderStages,::DataStream &ds) {push_constants(l,shader,shaderStages,0,ds,3);}
void Lua::Shader::BeginDraw(lua_State *l,::Shader::Base &shader)
{
	auto r = shader.BeginDraw();
	Lua::PushBool(l,r);
}
void Lua::Shader::EndDraw(lua_State*,::Shader::Base &shader)
{
	shader.EndDraw();
}
void Lua::Shader::BindDescriptorSet(lua_State *l,::Shader::Base &shader,uint32_t firstSet,Vulkan::DescriptorSetHandleOwner &hDescSet,uint32_t dynamicOffset)
{
	Lua::CheckVKDescriptorSet(l,3);
	auto &context = c_engine->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	auto &pipeline = *const_cast<::Vulkan::ShaderPipeline*>(shader.GetPipeline());

	auto layout = pipeline.GetPipelineLayout();
	drawCmd->BindDescriptorSet(firstSet,layout,hDescSet.get(),dynamicOffset);
}
void Lua::Shader::BindDescriptorSet(lua_State *l,::Shader::Base &shader,uint32_t firstSet,Vulkan::DescriptorSetHandleOwner &hDescSet)
{
	Lua::CheckVKDescriptorSet(l,3);
	auto &context = c_engine->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	auto &pipeline = *const_cast<::Vulkan::ShaderPipeline*>(shader.GetPipeline());

	auto layout = pipeline.GetPipelineLayout();
	drawCmd->BindDescriptorSet(firstSet,layout,hDescSet.get());
}
void Lua::Shader::GetDescriptorSet(lua_State *l,::Shader::Base &shader,uint32_t setIdx,uint32_t bindingPoint,uint32_t baseIndex)
{
	auto *descSet = const_cast<::Vulkan::ShaderPipeline*>(shader.GetPipeline())->GetDescriptorSet(setIdx,bindingPoint,baseIndex);
	if(descSet == nullptr)
		return;
	Lua::Push<Lua::Vulkan::DescriptorSetHandleOwner>(l,Lua::Vulkan::DescriptorSetHandleOwner((*descSet)->GetHandle(),*descSet));
}
void Lua::Shader::GetDescriptorSet(lua_State *l,::Shader::Base &shader,uint32_t setIdx,uint32_t bindingPoint)
{
	auto *descSet = const_cast<::Vulkan::ShaderPipeline*>(shader.GetPipeline())->GetDescriptorSet(setIdx,bindingPoint);
	if(descSet == nullptr)
		return;
	Lua::Push<Lua::Vulkan::DescriptorSetHandleOwner>(l,Lua::Vulkan::DescriptorSetHandleOwner((*descSet)->GetHandle(),*descSet));
}
void Lua::Shader::GetDescriptorSet(lua_State *l,::Shader::Base &shader,uint32_t setIdx)
{
	auto *descSet = const_cast<::Vulkan::ShaderPipeline*>(shader.GetPipeline())->GetDescriptorSet(setIdx);
	if(descSet == nullptr)
		return;
	Lua::Push<Lua::Vulkan::DescriptorSetHandleOwner>(l,Lua::Vulkan::DescriptorSetHandleOwner((*descSet)->GetHandle(),*descSet));
}
void Lua::Shader::SetBuffer(lua_State*,::Shader::Base &shader,uint32_t setIdx,::Vulkan::MultiBuffer &hBuffer,uint32_t bindingPoint)
{
	const_cast<::Vulkan::ShaderPipeline*>(shader.GetPipeline())->SetBuffer(setIdx,hBuffer,bindingPoint);
}
void Lua::Shader::SetBuffer(lua_State*,::Shader::Base &shader,uint32_t setIdx,::Vulkan::MultiBuffer &hBuffer)
{
	const_cast<::Vulkan::ShaderPipeline*>(shader.GetPipeline())->SetBuffer(setIdx,hBuffer);
}

///////////////////////////

void Lua::Shader::Debug::BeginDraw(lua_State *l,::Shader::Debug &shader,std::shared_ptr<::Camera> &cam)
{
	auto r = shader.BeginDraw(*cam);
	Lua::PushBool(l,r);
}
void Lua::Shader::Debug::Draw(lua_State *l,::Shader::Debug &shader,const Mat4 &m,Lua::Vulkan::DescriptorSetHandleOwner &hDescSet,Lua::Vulkan::BufferHandleOwner &hVertBuffer,uint32_t vertCount)
{
	Lua::CheckVKDescriptorSet(l,3);
	Lua::CheckVKBuffer(l,4);
	shader.Draw(m,hDescSet.get(),hVertBuffer.get(),vertCount);
}
void Lua::Shader::Debug::Draw(lua_State *l,::Shader::Debug &shader,const Mat4 &m,Lua::Vulkan::DescriptorSetHandleOwner &hDescSet,Lua::Vulkan::BufferHandleOwner &hVertBuffer,Lua::Vulkan::BufferHandleOwner &hColorBuffer,uint32_t vertCount)
{
	Lua::CheckVKDescriptorSet(l,3);
	Lua::CheckVKBuffer(l,4);
	Lua::CheckVKBuffer(l,5);
	shader.Draw(m,hDescSet.get(),hVertBuffer.get(),vertCount,hColorBuffer.get());
}

///////////////////////////

void Lua::Shader::TexturedBase3D::BindScene(lua_State *l,::Shader::TexturedBase3D &shader,std::shared_ptr<::Scene> &scene)
{
	shader.BindScene(*scene);
}
void Lua::Shader::TexturedBase3D::BindScene(lua_State *l,::Shader::TexturedBase3D &shader,std::shared_ptr<::Scene> &scene,bool bView)
{
	shader.BindScene(*scene,bView);
}
void Lua::Shader::TexturedBase3D::BindScene(lua_State *l,::Shader::TexturedBase3D &shader,std::shared_ptr<::Scene> &scene,bool bView,bool bLights)
{
	shader.BindScene(*scene,bView);
	if(bLights == true)
	{
		auto &descSetShadowmps = scene->GetLightShadowMapDescriptorSet();
		auto &descSetLightSources = scene->GetForwardPlusInstance().GetDescriptorSetGraphics();
		shader.BindLights(descSetShadowmps,descSetLightSources);
	}
}
void Lua::Shader::TexturedBase3D::BindEntity(lua_State *l,::Shader::Base3D &shader,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	auto &context = c_engine->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	auto bWeighted = false;
	auto r = shader.BindEntity(drawCmd,static_cast<CBaseEntity*>(hEnt.get()),0u,bWeighted);
	Lua::PushBool(l,r);
}
void Lua::Shader::TexturedBase3D::BindMaterial(lua_State*,::Shader::TexturedBase3D &shader,Material *mat)
{
	shader.BindMaterial(mat);
}
void Lua::Shader::TexturedBase3D::Draw(lua_State*,::Shader::Base3D &shader,std::shared_ptr<::ModelSubMesh> &mesh)
{
	shader.Draw(static_cast<CModelSubMesh*>(mesh.get()));
}
#endif