/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <misc/compute_pipeline_create_info.h>
#include <shader/prosper_shader_copy_image.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_util.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

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

static std::unordered_map<Anvil::BasePipelineCreateInfo*,LuaShaderBase*> s_pipelineToShader;
LuaShaderBase *LuaShaderBase::GetShader(Anvil::BasePipelineCreateInfo &pipelineInfo)
{
	auto it = s_pipelineToShader.find(&pipelineInfo);
	if(it == s_pipelineToShader.end())
		return nullptr;
	return it->second;
}
LuaShaderBase::LuaShaderBase(prosper::Shader &shader)
	: LuaObjectBase(),m_shader(shader)
{}
void LuaShaderBase::Initialize(const luabind::object &o)
{
	m_baseLuaObj = std::shared_ptr<luabind::object>(new luabind::object(o));
	CallLuaMember("Initialize");
}
void LuaShaderBase::ClearLuaObject() {m_baseLuaObj = nullptr;}
void LuaShaderBase::OnInitialized() {CallLuaMember("OnInitialized");}
void LuaShaderBase::OnPipelinesInitialized() {CallLuaMember("OnPipelinesInitialized");}
void LuaShaderBase::OnPipelineInitialized(uint32_t pipelineIdx) {CallLuaMember<void,uint32_t>("OnPipelineInitialized",pipelineIdx);}
void LuaShaderBase::InitializePipeline(Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	m_currentPipelineInfo = &pipelineInfo;
	m_currentDescSetIndex = 0u;

	auto it = s_pipelineToShader.insert(std::make_pair(&pipelineInfo,this)).first;
	CallLuaMember<void,std::reference_wrapper<Anvil::BasePipelineCreateInfo>,uint32_t>("InitializePipeline",std::ref(pipelineInfo),pipelineIdx);
	s_pipelineToShader.erase(it);

	m_currentPipelineInfo = nullptr;
	m_currentDescSetIndex = 0u;
}
bool LuaShaderBase::AttachDescriptorSetInfo(const pragma::LuaDescriptorSetInfo &descSetInfo)
{
	if(m_currentPipelineInfo == nullptr)
		return false;
	auto shaderDescSetInfo = to_prosper_descriptor_set_info(descSetInfo);
	shaderDescSetInfo.setIndex = (m_currentDescSetIndex != std::numeric_limits<uint32_t>::max()) ? m_currentDescSetIndex : descSetInfo.setIndex;
	m_currentDescSetIndex = shaderDescSetInfo.setIndex +1u;
	GetShader().AddDescriptorSetGroup(*m_currentPipelineInfo,shaderDescSetInfo);
	return true;
}
prosper::Shader &LuaShaderBase::GetShader() const {return m_shader;}

/////////////////

LuaShaderGraphicsBase::LuaShaderGraphicsBase(prosper::ShaderGraphics &shader)
	: LuaShaderBase(shader)
{}
bool LuaShaderGraphicsBase::AttachVertexAttribute(const pragma::LuaVertexBinding &binding,const std::vector<pragma::LuaVertexAttribute> &attributes)
{
	if(m_currentPipelineInfo == nullptr)
		return false;
	std::vector<Anvil::VertexInputAttribute> attributeData;
	attributeData.reserve(attributes.size());

	auto offset = 0u;
	for(auto &attr : attributes)
	{
		auto attrSize = prosper::util::get_byte_size(attr.format);
		auto attrOffset = (attr.offset != std::numeric_limits<uint32_t>::max()) ? attr.offset : offset;
		auto attrLocation = (attr.location != std::numeric_limits<uint32_t>::max()) ? attr.location : m_currentVertexAttributeLocation;
		attributeData.push_back({attrLocation,static_cast<Anvil::Format>(attr.format),attrOffset});

		offset = attrOffset +attrSize;
		++m_currentVertexAttributeLocation;
	}
	auto stride = (binding.stride != std::numeric_limits<uint32_t>::max()) ? binding.stride : offset;

	return static_cast<Anvil::GraphicsPipelineCreateInfo*>(m_currentPipelineInfo)->add_vertex_binding(
		m_currentVertexBinding++,
		static_cast<Anvil::VertexInputRate>(binding.inputRate),
		stride,
		attributeData.size(),
		attributeData.data()
	);
}
void LuaShaderGraphicsBase::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	m_currentVertexAttributeLocation = 0u;
	m_currentVertexBinding = 0u;

	InitializePipeline(pipelineInfo,pipelineIdx);

	m_currentVertexAttributeLocation = 0u;
	m_currentVertexBinding = 0u;
}
void LuaShaderGraphicsBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto ret = false;
	luabind::object r;
	if(CallLuaMember<luabind::object,uint32_t>("InitializeRenderPass",&r,pipelineIdx) == CallbackReturnType::HasReturnValue && r)
	{
		auto *l = r.interpreter();
		r.push(l); /* 1 */
		auto t = Lua::GetStackTop(l);
		Lua::CheckTable(l,t);

		Lua::PushInt(l,1); /* 2 */
		Lua::GetTableValue(l,t); /* 2 */
		outRenderPass = Lua::Check<Lua::Vulkan::RenderPass>(l,-1).shared_from_this();
		Lua::Pop(l,1); /* 1 */

		Lua::Pop(l,1); /* 0 */
		return;
	}
	InitializeDefaultRenderPass(outRenderPass,pipelineIdx);
}

/////////////////

LuaShaderComputeBase::LuaShaderComputeBase(prosper::ShaderCompute &shader)
	: LuaShaderBase(shader)
{}

void LuaShaderComputeBase::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {InitializePipeline(pipelineInfo,pipelineIdx);}

/////////////////

LuaShaderGraphics::LuaShaderGraphics()
	: TLuaShaderBase(*c_engine,"","","")
{}
void LuaShaderGraphics::Lua_InitializePipeline(Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderGraphics::InitializeGfxPipeline(static_cast<Anvil::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void LuaShaderGraphics::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);}
void LuaShaderGraphics::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeRenderPass(outRenderPass,pipelineIdx);}
void LuaShaderGraphics::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) {prosper::ShaderGraphics::InitializeRenderPass(outRenderPass,pipelineIdx);}

/////////////////

LuaShaderPostProcessing::LuaShaderPostProcessing()
	: TLuaShaderBase(*c_engine,"","","")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void LuaShaderPostProcessing::Lua_InitializePipeline(Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	pragma::ShaderPPBase::InitializeGfxPipeline(static_cast<Anvil::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}

void LuaShaderPostProcessing::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);}
void LuaShaderPostProcessing::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeRenderPass(outRenderPass,pipelineIdx);}
void LuaShaderPostProcessing::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) {pragma::ShaderPPBase::InitializeRenderPass(outRenderPass,pipelineIdx);}

/////////////////

LuaShaderImageProcessing::LuaShaderImageProcessing()
	: TLuaShaderBase(*c_engine,"","","")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void LuaShaderImageProcessing::Lua_InitializePipeline(Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderBaseImageProcessing::InitializeGfxPipeline(static_cast<Anvil::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}

void LuaShaderImageProcessing::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);}
void LuaShaderImageProcessing::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeRenderPass(outRenderPass,pipelineIdx);}
void LuaShaderImageProcessing::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) {prosper::ShaderBaseImageProcessing::InitializeRenderPass(outRenderPass,pipelineIdx);}

/////////////////

LuaShaderCompute::LuaShaderCompute()
	: TLuaShaderBase(*c_engine,"","")
{}
void LuaShaderCompute::Lua_InitializePipeline(Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(static_cast<Anvil::ComputePipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void LuaShaderCompute::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {LuaShaderComputeBase::InitializeComputePipeline(pipelineInfo,pipelineIdx);}

/////////////////

LuaShaderTextured3D::LuaShaderTextured3D()
	: TLuaShaderBase(*c_engine,"","","")
{
	// SetBaseShader<ShaderTextured3DBase>();
}
void LuaShaderTextured3D::Lua_InitializePipeline(Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipeline(static_cast<Anvil::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
bool LuaShaderTextured3D::Lua_BindMaterialParameters(Material &mat)
{
	return ShaderTextured3DBase::BindMaterialParameters(static_cast<CMaterial&>(mat));
}
void LuaShaderTextured3D::Lua_InitializeGfxPipelineVertexAttributes(Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipelineVertexAttributes(static_cast<Anvil::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void LuaShaderTextured3D::Lua_InitializeGfxPipelinePushConstantRanges(Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipelinePushConstantRanges(static_cast<Anvil::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void LuaShaderTextured3D::Lua_InitializeGfxPipelineDescriptorSets(Anvil::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipelineDescriptorSets(static_cast<Anvil::GraphicsPipelineCreateInfo&>(pipelineInfo),pipelineIdx);
}
void LuaShaderTextured3D::Lua_OnBindMaterial(Material &mat) {}
int32_t LuaShaderTextured3D::Lua_OnDraw(ModelSubMesh &mesh) {return umath::to_integral(util::EventReply::Unhandled);}
void LuaShaderTextured3D::Lua_OnBindEntity(EntityHandle &hEnt) {}
void LuaShaderTextured3D::Lua_OnBindScene(rendering::RasterizationRenderer &renderer,bool bView) {}
void LuaShaderTextured3D::Lua_OnBeginDraw(prosper::ICommandBuffer &drawCmd,const Vector4 &clipPlane,uint32_t pipelineIdx,uint32_t recordFlags) {}
void LuaShaderTextured3D::Lua_OnEndDraw() {}
bool LuaShaderTextured3D::BindMaterial(CMaterial &mat)
{
	if(ShaderTextured3DBase::BindMaterial(mat) == false)
		return false;
	CallLuaMember<void,Material*>("OnBindMaterial",&mat);
	return true;
}
bool LuaShaderTextured3D::Draw(CModelSubMesh &mesh)
{
	CallLuaMember<void,ModelSubMesh*>("OnDraw",&mesh);

	int32_t result = -1;
	if(CallLuaMember<int32_t,ModelSubMesh*>("OnDraw",&result,&mesh) == CallbackReturnType::HasReturnValue && static_cast<util::EventReply>(result) == util::EventReply::Handled)
		return true; // Skip default drawing
	return ShaderTextured3DBase::Draw(mesh);
}
bool LuaShaderTextured3D::BindEntity(CBaseEntity &ent)
{
	if(ShaderTextured3DBase::BindEntity(ent) == false)
		return false;
	auto &o = *ent.GetLuaObject();
	CallLuaMember<void,luabind::object>("OnBindEntity",o);
	return true;
}
bool LuaShaderTextured3D::BindVertexAnimationOffset(uint32_t offset)
{
	return ShaderTextured3DBase::BindVertexAnimationOffset(offset);
}
bool LuaShaderTextured3D::BindScene(rendering::RasterizationRenderer &renderer,bool bView)
{
	if(ShaderTextured3DBase::BindScene(renderer,bView) == false)
		return false;
	CallLuaMember<void,rendering::RasterizationRenderer*,bool>("OnBindScene",&renderer,bView);
	return true;
}
bool LuaShaderTextured3D::BeginDraw(
	const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,
	const Vector4 &drawOrigin,Pipeline pipelineIdx,RecordFlags recordFlags
)
{
	if(ShaderTextured3DBase::BeginDraw(cmdBuffer,clipPlane,drawOrigin,pipelineIdx,recordFlags) == false)
		return false;
	CallLuaMember<void,prosper::ICommandBuffer*,const Vector4&,uint32_t,uint32_t>("OnBeginDraw",const_cast<prosper::IPrimaryCommandBuffer*>(cmdBuffer.get()),clipPlane,umath::to_integral(pipelineIdx),umath::to_integral(recordFlags));
	return true;
}
void LuaShaderTextured3D::EndDraw()
{
	CallLuaMember<void>("OnEndDraw");
	ShaderTextured3DBase::EndDraw();
}
bool LuaShaderTextured3D::BindMaterialParameters(CMaterial &mat)
{
	auto ret = false;
	return CallLuaMember<bool,std::reference_wrapper<Material>>("BindMaterialParameters",&ret,std::ref(static_cast<Material&>(mat))) == CallbackReturnType::HasReturnValue || ret;
}
void LuaShaderTextured3D::InitializeGfxPipelineVertexAttributes(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	CallLuaMember<void,std::reference_wrapper<Anvil::BasePipelineCreateInfo>,uint32_t>("InitializeGfxPipelineVertexAttributes",std::ref(static_cast<Anvil::BasePipelineCreateInfo&>(pipelineInfo)),pipelineIdx);
}
void LuaShaderTextured3D::InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	CallLuaMember<void,std::reference_wrapper<Anvil::BasePipelineCreateInfo>,uint32_t>("InitializeGfxPipelinePushConstantRanges",std::ref(static_cast<Anvil::BasePipelineCreateInfo&>(pipelineInfo)),pipelineIdx);
}
void LuaShaderTextured3D::InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	CallLuaMember<void,std::reference_wrapper<Anvil::BasePipelineCreateInfo>,uint32_t>("InitializeGfxPipelineDescriptorSets",std::ref(static_cast<Anvil::BasePipelineCreateInfo&>(pipelineInfo)),pipelineIdx);
}
void LuaShaderTextured3D::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);}
void LuaShaderTextured3D::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeRenderPass(outRenderPass,pipelineIdx);}
void LuaShaderTextured3D::InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) {ShaderTextured3DBase::InitializeRenderPass(outRenderPass,pipelineIdx);}

/////////////////

static void get_descriptor_set_layout_bindings(lua_State *l,std::vector<prosper::DescriptorSetInfo::Binding> &bindings,int32_t tBindings)
{
	auto numBindings = Lua::GetObjectLength(l,tBindings);
	bindings.reserve(numBindings);
	for(auto j=decltype(numBindings){0};j<numBindings;++j)
	{
		Lua::PushInt(l,j +1); /* 1 */
		Lua::GetTableValue(l,tBindings);

		Lua::CheckTable(l,-1);
		auto tBinding = Lua::GetStackTop(l);

		auto type = prosper::DescriptorType::UniformBuffer;
		Lua::get_table_value<ptrdiff_t,decltype(type)>(l,"type",tBinding,type,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(Lua::CheckInt(l,idx));
		});

		auto shaderStages = prosper::ShaderStageFlags::AllGraphics;
		Lua::get_table_value<ptrdiff_t,decltype(shaderStages)>(l,"stage",tBinding,shaderStages,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(Lua::CheckInt(l,idx));
		});

		uint32_t arrayCount = 1;
		Lua::get_table_value<ptrdiff_t,decltype(arrayCount)>(l,"arrayCount",tBinding,arrayCount,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(Lua::CheckInt(l,idx));
		});

		prosper::DescriptorSetInfo::Binding binding {type,shaderStages,arrayCount};
		bindings.push_back(binding);

		Lua::Pop(l,1); /* 0 */
	}
}

LuaShaderManager::~LuaShaderManager()
{
	for(auto &pair : m_shaders)
	{
		if(pair.second.whShader.valid())
			c_engine->GetShaderManager().RemoveShader(*pair.second.whShader);
	}
}

void LuaShaderManager::RegisterShader(std::string className,luabind::object &o)
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
		Lua::HandleLuaError(l);
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
	auto *oShaderGraphics = luabind::object_cast_nothrow<pragma::LuaShaderGraphicsBase*>(r,static_cast<pragma::LuaShaderGraphicsBase*>(nullptr));
	auto *shader = oShaderGraphics ? dynamic_cast<LuaShaderBase*>(oShaderGraphics) : nullptr;
	if(shader == nullptr)
	{
		auto *oShaderCompute = luabind::object_cast_nothrow<pragma::LuaShaderComputeBase*>(r,static_cast<pragma::LuaShaderComputeBase*>(nullptr));
		shader = oShaderCompute ? dynamic_cast<LuaShaderBase*>(oShaderCompute) : nullptr;
		if(shader == nullptr)
		{
			Con::ccl<<"WARNING: Unable to create lua shader '"<<className<<"': Lua class is not derived from valid shader base!"<<Con::endl;
			return;
		}
		shaderType = ShaderType::Compute;
	}
	r.push(l); /* 1 */
	auto idx = Lua::GetStackTop(l);

	shader->Initialize(r);
	shader->SetIdentifier(className);

	if(shaderType == ShaderType::Graphics)
	{
		std::string fragmentShader;
		std::string vertexShader;
		std::string geometryShader;
		Lua::GetProtectedTableValue(l,idx,"FragmentShader",fragmentShader);
		Lua::GetProtectedTableValue(l,idx,"VertexShader",vertexShader);
		Lua::GetProtectedTableValue(l,idx,"GeometryShader",geometryShader);
		auto &shaderGraphics = *dynamic_cast<prosper::Shader*>(shader);
		if(fragmentShader.empty() == false)
			shaderGraphics.SetStageSourceFilePath(prosper::ShaderStage::Fragment,fragmentShader);
		if(vertexShader.empty() == false)
			shaderGraphics.SetStageSourceFilePath(prosper::ShaderStage::Vertex,vertexShader);
		if(geometryShader.empty() == false)
			shaderGraphics.SetStageSourceFilePath(prosper::ShaderStage::Geometry,geometryShader);
	}
	else
	{
		std::string computeShader;
		Lua::GetProtectedTableValue(l,idx,"ComputeShader",computeShader);
		auto &shaderCompute = *static_cast<pragma::LuaShaderCompute*>(shader);
		if(computeShader.empty() == false)
			shaderCompute.SetStageSourceFilePath(prosper::ShaderStage::Compute,computeShader);
	}
	Lua::Pop(l);

	pair.whShader = c_engine->GetShaderManager().RegisterShader(className,[shader](prosper::Context &context,const std::string &identifier,bool &bExternalOwnership) {
		bExternalOwnership = true;
		return dynamic_cast<prosper::Shader*>(shader);
	});
}

luabind::object *LuaShaderManager::GetClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_shaders.find(className);
	if(it == m_shaders.end())
		return nullptr;
	return &it->second.luaClassObject;
}
