/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/util.hpp>
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_util.hpp>
#include <wgui/wibase.h>

extern DLLCLIENT CEngine *c_engine;

prosper::DescriptorSetInfo pragma::to_prosper_descriptor_set_info(const LuaDescriptorSetInfo &descSetInfo)
{
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
	return shaderDescSetInfo;
}

static std::unordered_map<prosper::BasePipelineCreateInfo*,pragma::LuaShaderWrapperBase*> s_pipelineToShader;

pragma::LuaDescriptorSetInfo::LuaDescriptorSetInfo(luabind::object lbindings,uint32_t setIndex)
	: setIndex(setIndex)
{
	::Lua::get_table_values<LuaDescriptorSetBinding>(lbindings.interpreter(),2u,bindings,[](lua_State *l,int32_t idx) -> LuaDescriptorSetBinding {
		return *::Lua::CheckDescriptorSetBinding(l,idx);
	});
}

pragma::LuaShaderWrapperBase *pragma::LuaShaderWrapperBase::GetShader(prosper::BasePipelineCreateInfo &pipelineInfo)
{
	auto it = s_pipelineToShader.find(&pipelineInfo);
	if(it == s_pipelineToShader.end())
		return nullptr;
	return it->second;
}
pragma::LuaShaderWrapperBase::LuaShaderWrapperBase()
	: LuaObjectBase()
{}
void pragma::LuaShaderWrapperBase::Initialize(const luabind::object &o)
{
	m_baseLuaObj = std::shared_ptr<luabind::object>(new luabind::object(o));
	CallLuaMember("Initialize");
}
void pragma::LuaShaderWrapperBase::ClearLuaObject() {m_baseLuaObj = nullptr;}
void pragma::LuaShaderWrapperBase::OnInitialized() {CallLuaMember("OnInitialized");}
void pragma::LuaShaderWrapperBase::OnPipelinesInitialized() {CallLuaMember("OnPipelinesInitialized");}
void pragma::LuaShaderWrapperBase::OnPipelineInitialized(uint32_t pipelineIdx) {CallLuaMember<void,uint32_t>("OnPipelineInitialized",pipelineIdx);}
void pragma::LuaShaderWrapperBase::InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	m_currentPipelineInfo = &pipelineInfo;
	m_currentDescSetIndex = 0u;
	m_curPipelineIdx = pipelineIdx;

	auto it = s_pipelineToShader.insert(std::make_pair(&pipelineInfo,this)).first;
	CallLuaMember<void,std::reference_wrapper<prosper::BasePipelineCreateInfo>,uint32_t>("InitializePipeline",std::ref(pipelineInfo),pipelineIdx);
	s_pipelineToShader.erase(it);

	m_currentPipelineInfo = nullptr;
	m_currentDescSetIndex = 0u;
	m_curPipelineIdx = std::numeric_limits<uint32_t>::max();
}
bool pragma::LuaShaderWrapperBase::AttachDescriptorSetInfo(const pragma::LuaDescriptorSetInfo &descSetInfo,uint32_t pipelineIdx)
{
	if(m_currentPipelineInfo == nullptr)
		return false;
	auto shaderDescSetInfo = to_prosper_descriptor_set_info(descSetInfo);
	shaderDescSetInfo.setIndex = (m_currentDescSetIndex != std::numeric_limits<uint32_t>::max()) ? m_currentDescSetIndex : descSetInfo.setIndex;
	m_currentDescSetIndex = shaderDescSetInfo.setIndex +1u;
	GetShader().AddDescriptorSetGroup(*m_currentPipelineInfo,pipelineIdx,shaderDescSetInfo);
	return true;
}
void pragma::LuaShaderWrapperBase::SetStageSourceFilePath(prosper::ShaderStage shaderStage,const std::string &fpath)
{
	m_shader->SetStageSourceFilePath(shaderStage,fpath);
}
void pragma::LuaShaderWrapperBase::SetPipelineCount(uint32_t pipelineCount)
{
	dynamic_cast<LShaderBase*>(m_shader)->SetPipelineCount(pipelineCount);
}
prosper::Shader &pragma::LuaShaderWrapperBase::GetShader() const {return *m_shader;}
void pragma::LuaShaderWrapperBase::SetShader(prosper::Shader *shader)
{
	m_shader = shader;
	if(shader)
	{
		// No multi-threading for Lua callbacks
		shader->SetMultiThreadedPipelineInitializationEnabled(false);
	}
}

/////////////////

void pragma::LShaderGraphics::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperGraphics*>(m_wrapper)->InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}
void pragma::LShaderGraphics::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperGraphics*>(m_wrapper)->InitializeRenderPass(outRenderPass,pipelineIdx);
}
pragma::LuaShaderWrapperGraphicsBase::LuaShaderWrapperGraphicsBase()
	: pragma::LuaShaderWrapperBase()
{}
bool pragma::LuaShaderWrapperGraphicsBase::AttachVertexAttribute(const pragma::LuaVertexBinding &binding,const std::vector<pragma::LuaVertexAttribute> &attributes)
{
	if(m_currentPipelineInfo == nullptr)
		return false;
	std::vector<prosper::VertexInputAttribute> attributeData;
	attributeData.reserve(attributes.size());

	auto offset = 0u;
	for(auto &attr : attributes)
	{
		auto attrSize = prosper::util::get_byte_size(attr.format);
		auto attrOffset = (attr.offset != std::numeric_limits<uint32_t>::max()) ? attr.offset : offset;
		auto attrLocation = (attr.location != std::numeric_limits<uint32_t>::max()) ? attr.location : m_currentVertexAttributeLocation;
		attributeData.push_back({attrLocation,static_cast<prosper::Format>(attr.format),attrOffset});

		offset = attrOffset +attrSize;
		++m_currentVertexAttributeLocation;
	}
	auto stride = (binding.stride != std::numeric_limits<uint32_t>::max()) ? binding.stride : offset;

	return static_cast<prosper::GraphicsPipelineCreateInfo*>(m_currentPipelineInfo)->AddVertexBinding(
		m_currentVertexBinding++,
		static_cast<prosper::VertexInputRate>(binding.inputRate),
		stride,
		attributeData.size(),
		attributeData.data()
	);
}
void pragma::LuaShaderWrapperGraphicsBase::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	m_currentVertexAttributeLocation = 0u;
	m_currentVertexBinding = 0u;

	InitializePipeline(pipelineInfo,pipelineIdx);

	m_currentVertexAttributeLocation = 0u;
	m_currentVertexBinding = 0u;
}
void pragma::LuaShaderWrapperGraphicsBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto ret = false;
	luabind::object r;
	if(CallLuaMember<luabind::object,uint32_t>("InitializeRenderPass",&r,pipelineIdx) == CallbackReturnType::HasReturnValue && r)
	{
		auto *l = r.interpreter();
		r.push(l); /* 1 */
		auto t = ::Lua::GetStackTop(l);
		::Lua::CheckTable(l,t);

		::Lua::PushInt(l,1); /* 2 */
		::Lua::GetTableValue(l,t); /* 2 */
		outRenderPass = ::Lua::Check<::Lua::Vulkan::RenderPass>(l,-1).shared_from_this();
		::Lua::Pop(l,1); /* 1 */

		::Lua::Pop(l,1); /* 0 */
		return;
	}
	InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

pragma::LuaShaderWrapperComputeBase::LuaShaderWrapperComputeBase()
	: pragma::LuaShaderWrapperBase()
{}

void pragma::LuaShaderWrapperComputeBase::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {InitializePipeline(pipelineInfo,pipelineIdx);}

/////////////////

pragma::LShaderGraphics::LShaderGraphics()
	: TLShaderBase<prosper::ShaderGraphics>(c_engine->GetRenderContext(),"","","")
{}
void pragma::LShaderGraphics::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	prosper::ShaderGraphics::InitializeRenderPass(outRenderPass,pipelineIdx);
}
pragma::LuaShaderWrapperGraphics::LuaShaderWrapperGraphics()
{}
void pragma::LuaShaderWrapperGraphics::Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderGraphics*>(m_shader)->BaseInitializeGfxPipeline(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperGraphics::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LShaderGraphics*>(m_shader)->InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

pragma::LShaderCompute::LShaderCompute()
	: TLShaderBase<prosper::ShaderCompute>(c_engine->GetRenderContext(),"","")
{}
pragma::LuaShaderWrapperCompute::LuaShaderWrapperCompute()
{}
void pragma::LuaShaderWrapperCompute::Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderCompute*>(m_shader)->BaseInitializeComputePipeline(static_cast<prosper::ComputePipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}

/////////////////

void pragma::LShaderCompute::InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperCompute*>(m_wrapper)->InitializeComputePipeline(pipelineInfo,pipelineIdx);
}
pragma::LShaderGui::LShaderGui()
	: TLShaderBase<wgui::Shader>(c_engine->GetRenderContext(),"","","")
{
	SetPipelineCount(umath::to_integral(wgui::StencilPipeline::Count) *2);
}
void pragma::LShaderGui::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperGUI*>(m_wrapper)->InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}
void pragma::LShaderGui::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperGUI*>(m_wrapper)->InitializeRenderPass(outRenderPass,pipelineIdx);
}
bool pragma::LShaderGui::RecordBeginDraw(
	prosper::ShaderBindState &bindState,wgui::DrawState &drawState,uint32_t width,uint32_t height,
	wgui::StencilPipeline pipelineIdx,bool msaa,uint32_t testStencilLevel
) const
{
	return wgui::Shader::RecordBeginDraw(bindState,drawState,width,height,pipelineIdx,msaa) &&
		RecordSetStencilReference(bindState,testStencilLevel);
}
void pragma::LShaderGui::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	wgui::Shader::InitializeRenderPass(outRenderPass,pipelineIdx);
}
pragma::LuaShaderWrapperGUI::LuaShaderWrapperGUI()
{}

void pragma::LuaShaderWrapperGUI::Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderGui*>(m_shader)->BaseInitializeGfxPipeline(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperGUI::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LShaderGui*>(m_shader)->InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

pragma::LShaderGuiTextured::LShaderGuiTextured()
	: TLShaderBase<wgui::ShaderTextured>(c_engine->GetRenderContext(),"","","")
{}
void pragma::LShaderGuiTextured::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperGUITextured*>(m_wrapper)->InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}
void pragma::LShaderGuiTextured::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperGUITextured*>(m_wrapper)->InitializeRenderPass(outRenderPass,pipelineIdx);
}
void pragma::LShaderGuiTextured::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	wgui::ShaderTextured::InitializeRenderPass(outRenderPass,pipelineIdx);
}
pragma::LuaShaderWrapperGUITextured::LuaShaderWrapperGUITextured()
{}

void pragma::LuaShaderWrapperGUITextured::Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderGuiTextured*>(m_shader)->BaseInitializeGfxPipeline(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperGUITextured::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LShaderGuiTextured*>(m_shader)->InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

pragma::LShaderParticle2D::LShaderParticle2D()
	: TLShaderBase<pragma::ShaderParticle2DBase>(c_engine->GetRenderContext(),"","","")
{}
Vector3 pragma::LShaderParticle2D::DoCalcVertexPosition(
	const pragma::CParticleSystemComponent &ptc,uint32_t ptIdx,uint32_t localVertIdx,
	const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
) const
{
	return static_cast<LuaShaderWrapperParticle2D*>(m_wrapper)->DoCalcVertexPosition(ptc,ptIdx,localVertIdx,camPos,camUpWs,camRightWs,nearZ,farZ);
}
void pragma::LShaderParticle2D::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperParticle2D*>(m_wrapper)->InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}
void pragma::LShaderParticle2D::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperParticle2D*>(m_wrapper)->InitializeRenderPass(outRenderPass,pipelineIdx);
}
void pragma::LShaderParticle2D::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	pragma::ShaderParticle2DBase::InitializeRenderPass(outRenderPass,pipelineIdx);
}
pragma::LuaShaderWrapperParticle2D::LuaShaderWrapperParticle2D()
{}
Vector3 pragma::LuaShaderWrapperParticle2D::Lua_CalcVertexPosition(
	lua_State *l,pragma::CParticleSystemComponent &hPtC,uint32_t ptIdx,uint32_t localVertIdx,const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
)
{
	return static_cast<LShaderParticle2D*>(m_shader)->DoCalcVertexPosition(hPtC,ptIdx,localVertIdx,camPos,camUpWs,camRightWs,nearZ,farZ);
}
void pragma::LuaShaderWrapperParticle2D::Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderParticle2D*>(m_shader)->BaseInitializeGfxPipeline(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}

void pragma::reset_lua_shaders()
{
	auto &shaderManager = c_engine->GetShaderManager();
	for(auto &shader : shaderManager.GetShaders())
	{
		auto *ls = dynamic_cast<pragma::LShaderBase*>(shader.get());
		if(ls)
			ls->SetLuaShader({},nullptr);
	}
}
Vector3 pragma::LuaShaderWrapperParticle2D::DoCalcVertexPosition(
	const pragma::CParticleSystemComponent &ptc,uint32_t ptIdx,uint32_t localVertIdx,
	const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
) const
{
	auto ret = false;
	luabind::object r;
	auto &lPtc = ptc.GetLuaObject();
	if(const_cast<pragma::LuaShaderWrapperParticle2D*>(this)->CallLuaMember<
		luabind::object,luabind::object,uint32_t,uint32_t,const Vector3&,const Vector3&,const Vector3&,float,float
	>("CalcVertexPosition",&r,lPtc,ptIdx,localVertIdx,camPos,camUpWs,camRightWs,nearZ,farZ) == CallbackReturnType::HasReturnValue && r)
	{
		auto *l = r.interpreter();
		r.push(l); /* 1 */
		auto &v = ::Lua::Check<Vector3>(l,-1);
		::Lua::Pop(l,1);
		return v;
	}
	return static_cast<LShaderParticle2D*>(m_shader)->DoCalcVertexPosition(ptc,ptIdx,localVertIdx,camPos,camUpWs,camRightWs,nearZ,farZ);
}

void pragma::LuaShaderWrapperParticle2D::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LShaderParticle2D*>(m_shader)->InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

pragma::LShaderImageProcessing::LShaderImageProcessing()
	: TLShaderBase<prosper::ShaderBaseImageProcessing>(c_engine->GetRenderContext(),"","","")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}
void pragma::LShaderImageProcessing::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperImageProcessing*>(m_wrapper)->InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}
void pragma::LShaderImageProcessing::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperImageProcessing*>(m_wrapper)->InitializeRenderPass(outRenderPass,pipelineIdx);
}
void pragma::LShaderImageProcessing::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	prosper::ShaderBaseImageProcessing::InitializeRenderPass(outRenderPass,pipelineIdx);
}
pragma::LuaShaderWrapperImageProcessing::LuaShaderWrapperImageProcessing()
{}

void pragma::LuaShaderWrapperImageProcessing::Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderImageProcessing*>(m_shader)->BaseInitializeGfxPipeline(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}

void pragma::LuaShaderWrapperImageProcessing::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LShaderImageProcessing*>(m_shader)->InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

pragma::LShaderPostProcessing::LShaderPostProcessing()
	: TLShaderBase<ShaderPPBase>(c_engine->GetRenderContext(),"","","")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}
void pragma::LShaderPostProcessing::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	ShaderPPBase::InitializeRenderPass(outRenderPass,pipelineIdx);
}
pragma::LuaShaderWrapperPostProcessing::LuaShaderWrapperPostProcessing()
{}

void pragma::LuaShaderWrapperPostProcessing::Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderPostProcessing*>(m_shader)->BaseInitializeGfxPipeline(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}

void pragma::LuaShaderWrapperPostProcessing::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LShaderPostProcessing*>(m_shader)->InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

pragma::LShaderGameWorldLightingPass::LShaderGameWorldLightingPass()
	: TLShaderBase<ShaderGameWorldLightingPass>(c_engine->GetRenderContext(),"","","")
{
	// SetBaseShader<ShaderTextured3DBase>();
}
void pragma::LShaderGameWorldLightingPass::BaseInitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::BaseInitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeRenderPass(outRenderPass,pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::BaseInitializeGfxPipelineVertexAttributes(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineVertexAttributes(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::BaseInitializeGfxPipelinePushConstantRanges(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipelinePushConstantRanges(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::BaseInitializeGfxPipelineDescriptorSets(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperTextured3D*>(m_wrapper)->InitializeGfxPipelineVertexAttributes(pipelineInfo,pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperTextured3D*>(m_wrapper)->InitializeGfxPipelinePushConstantRanges(pipelineInfo,pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperTextured3D*>(m_wrapper)->InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperTextured3D*>(m_wrapper)->InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperTextured3D*>(m_wrapper)->InitializeRenderPass(outRenderPass,pipelineIdx);
}
void pragma::LShaderGameWorldLightingPass::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeRenderPass(outRenderPass,pipelineIdx);
}
pragma::LuaShaderWrapperTextured3D::LuaShaderWrapperTextured3D()
{}
void pragma::LuaShaderWrapperTextured3D::Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderGameWorldLightingPass*>(m_shader)->BaseInitializeGfxPipeline(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperTextured3D::Lua_InitializeGfxPipelineVertexAttributes(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderGameWorldLightingPass*>(m_shader)->BaseInitializeGfxPipelineVertexAttributes(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperTextured3D::Lua_InitializeGfxPipelinePushConstantRanges(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderGameWorldLightingPass*>(m_shader)->BaseInitializeGfxPipelinePushConstantRanges(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperTextured3D::Lua_InitializeGfxPipelineDescriptorSets(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderGameWorldLightingPass*>(m_shader)->BaseInitializeGfxPipelineDescriptorSets(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperTextured3D::Lua_OnBindMaterial(Material &mat) {}
int32_t pragma::LuaShaderWrapperTextured3D::Lua_OnDraw(ModelSubMesh &mesh) {return umath::to_integral(util::EventReply::Unhandled);}
void pragma::LuaShaderWrapperTextured3D::Lua_OnBindEntity(EntityHandle &hEnt) {}
void pragma::LuaShaderWrapperTextured3D::Lua_OnBindScene(CRasterizationRendererComponent &renderer,bool bView) {}
void pragma::LuaShaderWrapperTextured3D::Lua_OnBeginDraw(prosper::ICommandBuffer &drawCmd,const Vector4 &clipPlane,uint32_t pipelineIdx,uint32_t recordFlags) {}
void pragma::LuaShaderWrapperTextured3D::Lua_OnEndDraw() {}
void pragma::LuaShaderWrapperTextured3D::InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	CallLuaMember<void,std::reference_wrapper<prosper::BasePipelineCreateInfo>,uint32_t>("InitializeGfxPipelineVertexAttributes",std::ref(static_cast<prosper::BasePipelineCreateInfo&>(pipelineInfo)),pipelineIdx);
}
void pragma::LuaShaderWrapperTextured3D::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	CallLuaMember<void,std::reference_wrapper<prosper::BasePipelineCreateInfo>,uint32_t>("InitializeGfxPipelinePushConstantRanges",std::ref(static_cast<prosper::BasePipelineCreateInfo&>(pipelineInfo)),pipelineIdx);
}
void pragma::LuaShaderWrapperTextured3D::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	CallLuaMember<void,std::reference_wrapper<prosper::BasePipelineCreateInfo>,uint32_t>("InitializeGfxPipelineDescriptorSets",std::ref(static_cast<prosper::BasePipelineCreateInfo&>(pipelineInfo)),pipelineIdx);
}
void pragma::LuaShaderWrapperTextured3D::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LShaderGameWorldLightingPass*>(m_shader)->InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

pragma::LShaderPbr::LShaderPbr()
	: TLShaderBase<ShaderPBR>(c_engine->GetRenderContext(),"","","")
{
	// SetBaseShader<ShaderTextured3DBase>();
}
pragma::LuaShaderWrapperPbr::LuaShaderWrapperPbr()
{}
void pragma::LShaderPbr::InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperPbr*>(m_wrapper)->InitializeGfxPipelineVertexAttributes(pipelineInfo,pipelineIdx);
}
void pragma::LShaderPbr::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperPbr*>(m_wrapper)->InitializeGfxPipelinePushConstantRanges(pipelineInfo,pipelineIdx);
}
void pragma::LShaderPbr::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperPbr*>(m_wrapper)->InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
}
void pragma::LShaderPbr::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperPbr*>(m_wrapper)->InitializeGfxPipeline(pipelineInfo,pipelineIdx);
}
void pragma::LShaderPbr::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LuaShaderWrapperPbr*>(m_wrapper)->InitializeRenderPass(outRenderPass,pipelineIdx);
}
void pragma::LShaderPbr::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	ShaderPBR::InitializeRenderPass(outRenderPass,pipelineIdx);
}
void pragma::LuaShaderWrapperPbr::Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderPbr*>(m_shader)->BaseInitializeGfxPipeline(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperPbr::Lua_InitializeGfxPipelineVertexAttributes(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderPbr*>(m_shader)->BaseInitializeGfxPipelineVertexAttributes(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperPbr::Lua_InitializeGfxPipelinePushConstantRanges(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderPbr*>(m_shader)->BaseInitializeGfxPipelinePushConstantRanges(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperPbr::Lua_InitializeGfxPipelineDescriptorSets(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	static_cast<LShaderPbr*>(m_shader)->BaseInitializeGfxPipelineDescriptorSets(static_cast<prosper::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void pragma::LuaShaderWrapperPbr::Lua_OnBindMaterial(Material &mat) {}
int32_t pragma::LuaShaderWrapperPbr::Lua_OnDraw(ModelSubMesh &mesh) {return umath::to_integral(util::EventReply::Unhandled);}
void pragma::LuaShaderWrapperPbr::Lua_OnBindEntity(EntityHandle &hEnt) {}
void pragma::LuaShaderWrapperPbr::Lua_OnBindScene(CRasterizationRendererComponent &renderer,bool bView) {}
void pragma::LuaShaderWrapperPbr::Lua_OnBeginDraw(prosper::ICommandBuffer &drawCmd,const Vector4 &clipPlane,uint32_t pipelineIdx,uint32_t recordFlags) {}
void pragma::LuaShaderWrapperPbr::Lua_OnEndDraw() {}
void pragma::LuaShaderWrapperPbr::InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	CallLuaMember<void,std::reference_wrapper<prosper::BasePipelineCreateInfo>,uint32_t>("InitializeGfxPipelineVertexAttributes",std::ref(static_cast<prosper::BasePipelineCreateInfo&>(pipelineInfo)),pipelineIdx);
}
void pragma::LuaShaderWrapperPbr::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	CallLuaMember<void,std::reference_wrapper<prosper::BasePipelineCreateInfo>,uint32_t>("InitializeGfxPipelinePushConstantRanges",std::ref(static_cast<prosper::BasePipelineCreateInfo&>(pipelineInfo)),pipelineIdx);
}
void pragma::LuaShaderWrapperPbr::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	CallLuaMember<void,std::reference_wrapper<prosper::BasePipelineCreateInfo>,uint32_t>("InitializeGfxPipelineDescriptorSets",std::ref(static_cast<prosper::BasePipelineCreateInfo&>(pipelineInfo)),pipelineIdx);
}
void pragma::LuaShaderWrapperPbr::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	static_cast<LShaderPbr*>(m_shader)->InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

static void get_descriptor_set_layout_bindings(lua_State *l,std::vector<prosper::DescriptorSetInfo::Binding> &bindings,int32_t tBindings)
{
	auto numBindings = ::Lua::GetObjectLength(l,tBindings);
	bindings.reserve(numBindings);
	for(auto j=decltype(numBindings){0};j<numBindings;++j)
	{
		::Lua::PushInt(l,j +1); /* 1 */
		::Lua::GetTableValue(l,tBindings);

		::Lua::CheckTable(l,-1);
		auto tBinding = ::Lua::GetStackTop(l);

		auto type = prosper::DescriptorType::UniformBuffer;
		::Lua::get_table_value<ptrdiff_t,decltype(type)>(l,"type",tBinding,type,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(::Lua::CheckInt(l,idx));
		});

		auto shaderStages = prosper::ShaderStageFlags::AllGraphics;
		::Lua::get_table_value<ptrdiff_t,decltype(shaderStages)>(l,"stage",tBinding,shaderStages,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(::Lua::CheckInt(l,idx));
		});

		uint32_t arrayCount = 1;
		::Lua::get_table_value<ptrdiff_t,decltype(arrayCount)>(l,"arrayCount",tBinding,arrayCount,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(::Lua::CheckInt(l,idx));
		});

		prosper::DescriptorSetInfo::Binding binding {type,shaderStages,arrayCount};
		bindings.push_back(binding);

		::Lua::Pop(l,1); /* 0 */
	}
}

pragma::LuaShaderManager::~LuaShaderManager() {}

void pragma::LuaShaderManager::RegisterShader(std::string className,luabind::object &o)
{
	ustring::to_lower(className);
	auto itShader = m_shaders.find(className);
	if(itShader != m_shaders.end())
	{
		Con::cwar<<"WARNING: Attempted to register shader '"<<className<<"', which has already been registered previously! Ignoring..."<<Con::endl;
		return;
	}
	auto *l = o.interpreter();
	luabind::object r;
#ifndef LUABIND_NO_EXCEPTIONS
	try
	{
#endif
		r = o();
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error&)
	{
		::Lua::HandleLuaError(l);
		return;
	}
#endif
	if(!r)
	{
		Con::ccl<<"WARNING: Unable to create lua shader '"<<className<<"'!"<<Con::endl;
		return;
	}
	
	auto &pair = m_shaders[className] = {};
	pair.luaClassObject = o;

	enum class ShaderType : uint8_t
	{
		Graphics = 0,
		Compute
	};
	auto shaderType = ShaderType::Graphics;
	auto *oShaderGraphics = luabind::object_cast_nothrow<pragma::LuaShaderWrapperGraphicsBase*>(r,static_cast<pragma::LuaShaderWrapperGraphicsBase*>(nullptr));
	auto *shaderWrapper = oShaderGraphics ? dynamic_cast<pragma::LuaShaderWrapperBase*>(oShaderGraphics) : nullptr;
	if(shaderWrapper == nullptr)
	{
		auto *oShaderCompute = luabind::object_cast_nothrow<pragma::LuaShaderWrapperComputeBase*>(r,static_cast<pragma::LuaShaderWrapperComputeBase*>(nullptr));
		shaderWrapper = oShaderCompute ? dynamic_cast<pragma::LuaShaderWrapperBase*>(oShaderCompute) : nullptr;
		if(shaderWrapper == nullptr)
		{
			Con::ccl<<"WARNING: Unable to create lua shader '"<<className<<"': Lua class is not derived from valid shader base!"<<Con::endl;
			return;
		}
		shaderType = ShaderType::Compute;
	}
	r.push(l); /* 1 */
	auto idx = ::Lua::GetStackTop(l);

	auto &shaderManager = c_engine->GetShaderManager();
	auto shader = shaderManager.GetShader(className);
	if(shader.expired())
	{
		auto *newLShader = shaderWrapper->CreateShader();
		assert(newLShader != nullptr);
		if(!newLShader)
			throw std::runtime_error{"Failed to instantiate new Lua shader!"};
		auto *newShader = dynamic_cast<prosper::Shader*>(newLShader);
		assert(newShader != nullptr);
		if(!newShader)
			throw std::runtime_error{"Instantiated Lua shader is not a valid shader!"};
		
		newLShader->SetIdentifier(className);
		shaderWrapper->SetShader(newShader);
		shaderWrapper->Initialize(r);
		if(shaderType == ShaderType::Graphics)
		{
			std::string fragmentShader;
			std::string vertexShader;
			std::string geometryShader;
			::Lua::GetProtectedTableValue(l,idx,"FragmentShader",fragmentShader);
			::Lua::GetProtectedTableValue(l,idx,"VertexShader",vertexShader);
			::Lua::GetProtectedTableValue(l,idx,"GeometryShader",geometryShader);
			if(fragmentShader.empty() == false)
				newShader->SetStageSourceFilePath(prosper::ShaderStage::Fragment,fragmentShader);
			if(vertexShader.empty() == false)
				newShader->SetStageSourceFilePath(prosper::ShaderStage::Vertex,vertexShader);
			if(geometryShader.empty() == false)
				newShader->SetStageSourceFilePath(prosper::ShaderStage::Geometry,geometryShader);
		}
		else
		{
			std::string computeShader;
			::Lua::GetProtectedTableValue(l,idx,"ComputeShader",computeShader);
			auto &shaderCompute = *static_cast<pragma::LuaShaderWrapperCompute*>(shaderWrapper);
			if(computeShader.empty() == false)
				newShader->SetStageSourceFilePath(prosper::ShaderStage::Compute,computeShader);
		}

		shaderManager.RegisterShader(className,[newShader](prosper::IPrContext &context,const std::string &identifier) {
			return newShader;
		});
		shader = shaderManager.GetShader(className);
		assert(!shader.expired());
		if(shader.expired())
		{
			delete newShader;
			throw std::runtime_error{"Failed to register new Lua shader!"};
		}
	}
	else
	{
		shaderWrapper->SetShader(shader.get());
		shaderWrapper->Initialize(r);
	}
	::Lua::Pop(l);
	pair.whShader = shaderManager.GetShader(className);
	assert(pair.whShader.valid());
	dynamic_cast<LShaderBase*>(pair.whShader.get())->SetLuaShader(r,shaderWrapper);
}

luabind::object *pragma::LuaShaderManager::GetClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_shaders.find(className);
	if(it == m_shaders.end())
		return nullptr;
	return &it->second.luaClassObject;
}
