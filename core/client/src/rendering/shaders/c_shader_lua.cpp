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

prosper::Shader::DescriptorSetInfo pragma::to_prosper_descriptor_set_info(const LuaDescriptorSetInfo &descSetInfo)
{
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
		attributeData.push_back({attrLocation,attr.format,attrOffset});

		offset = attrOffset +attrSize;
		++m_currentVertexAttributeLocation;
	}
	auto stride = (binding.stride != std::numeric_limits<uint32_t>::max()) ? binding.stride : offset;

	return static_cast<Anvil::GraphicsPipelineCreateInfo*>(m_currentPipelineInfo)->add_vertex_binding(
		m_currentVertexBinding++,
		binding.inputRate,
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
void LuaShaderGraphicsBase::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
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
void LuaShaderGraphics::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeRenderPass(outRenderPass,pipelineIdx);}
void LuaShaderGraphics::InitializeDefaultRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) {prosper::ShaderGraphics::InitializeRenderPass(outRenderPass,pipelineIdx);}

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
void LuaShaderPostProcessing::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeRenderPass(outRenderPass,pipelineIdx);}
void LuaShaderPostProcessing::InitializeDefaultRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) {pragma::ShaderPPBase::InitializeRenderPass(outRenderPass,pipelineIdx);}

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
void LuaShaderImageProcessing::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeRenderPass(outRenderPass,pipelineIdx);}
void LuaShaderImageProcessing::InitializeDefaultRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) {prosper::ShaderBaseImageProcessing::InitializeRenderPass(outRenderPass,pipelineIdx);}

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
	SetBaseShader<ShaderTextured3D>();
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
void LuaShaderTextured3D::Lua_OnBeginDraw(prosper::CommandBuffer &drawCmd,const Vector4 &clipPlane,uint32_t pipelineIdx,uint32_t recordFlags) {}
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
	const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,Pipeline pipelineIdx,RecordFlags recordFlags
)
{
	if(ShaderTextured3DBase::BeginDraw(cmdBuffer,clipPlane,pipelineIdx,recordFlags) == false)
		return false;
	CallLuaMember<void,prosper::CommandBuffer*,const Vector4&,uint32_t,uint32_t>("OnBeginDraw",const_cast<prosper::PrimaryCommandBuffer*>(cmdBuffer.get()),clipPlane,umath::to_integral(pipelineIdx),umath::to_integral(recordFlags));
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
void LuaShaderTextured3D::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) {LuaShaderGraphicsBase::InitializeRenderPass(outRenderPass,pipelineIdx);}
void LuaShaderTextured3D::InitializeDefaultRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) {ShaderTextured3DBase::InitializeRenderPass(outRenderPass,pipelineIdx);}

/////////////////

static void get_descriptor_set_layout_bindings(lua_State *l,std::vector<prosper::Shader::DescriptorSetInfo::Binding> &bindings,int32_t tBindings)
{
	auto numBindings = Lua::GetObjectLength(l,tBindings);
	bindings.reserve(numBindings);
	for(auto j=decltype(numBindings){0};j<numBindings;++j)
	{
		Lua::PushInt(l,j +1); /* 1 */
		Lua::GetTableValue(l,tBindings);

		Lua::CheckTable(l,-1);
		auto tBinding = Lua::GetStackTop(l);

		auto type = Anvil::DescriptorType::UNIFORM_BUFFER;
		Lua::get_table_value<ptrdiff_t,decltype(type)>(l,"type",tBinding,type,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(Lua::CheckInt(l,idx));
		});

		auto shaderStages = Anvil::ShaderStageFlagBits::ALL_GRAPHICS;
		Lua::get_table_value<ptrdiff_t,decltype(shaderStages)>(l,"stage",tBinding,shaderStages,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(Lua::CheckInt(l,idx));
		});

		uint32_t arrayCount = 1;
		Lua::get_table_value<ptrdiff_t,decltype(arrayCount)>(l,"arrayCount",tBinding,arrayCount,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(Lua::CheckInt(l,idx));
		});

		prosper::Shader::DescriptorSetInfo::Binding binding {type,shaderStages,arrayCount};
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
	auto bOverwrite = true;

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
			shaderGraphics.SetStageSourceFilePath(Anvil::ShaderStage::FRAGMENT,fragmentShader);
		if(vertexShader.empty() == false)
			shaderGraphics.SetStageSourceFilePath(Anvil::ShaderStage::VERTEX,vertexShader);
		if(geometryShader.empty() == false)
			shaderGraphics.SetStageSourceFilePath(Anvil::ShaderStage::GEOMETRY,geometryShader);
	}
	else
	{
		std::string computeShader;
		Lua::GetProtectedTableValue(l,idx,"ComputeShader",computeShader);
		auto &shaderCompute = *static_cast<pragma::LuaShaderCompute*>(shader);
		if(computeShader.empty() == false)
			shaderCompute.SetStageSourceFilePath(Anvil::ShaderStage::COMPUTE,computeShader);
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


#if 0
LuaShaderBase::LuaShaderBase(const luabind::object &o)
	: LuaObjectBase(o)
{}
void LuaShaderBase::Initialize(const luabind::object &o)
{
	m_baseLuaObj = std::shared_ptr<luabind::object>(new luabind::object(o));
}

template<class TShaderBase>
	ShaderLuaBase<TShaderBase>::ShaderLuaBase(const luabind::object &o,prosper::Context &context,const std::string &identifier)
		: LuaShaderBase(o),TShaderBase(context,identifier,"")//,"","") // TODO
{}

template<class TShaderBase>
	void ShaderLuaBase<TShaderBase>::ClearLuaObject() {m_baseLuaObj = nullptr;}
/*template<class TShaderBase>
	void ShaderLuaBase<TShaderBase>::Initialize(
		const luabind::object &o,vk::PrimitiveTopology topology,const std::vector<vk::DynamicState> &dynamicStates,
		const std::vector<vk::PushConstantRange> &pushConstantRanges,const std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,
		const std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,const std::vector<prosper::Shader::DescriptorSetInfo> &descriptorSetLayouts,
		const LuaVKPipelineRasterizationStateCreateInfo &rasterizationState,const vk::PipelineDepthStencilStateCreateInfo &depthStencilInfo,
		const std::string &vsShader,const std::string &fsShader,const std::string &gsShader
)
{
	LuaShaderBase::Initialize(o);
	m_initData = std::unique_ptr<InitData>(new InitData());
	m_initData->topology = topology;
	m_initData->dynamicStates = dynamicStates;
	m_initData->pushConstantRanges = pushConstantRanges;
	m_initData->vertexBindingDescriptions = vertexBindingDescriptions;
	m_initData->vertexAttributeDescriptions = vertexAttributeDescriptions;
	m_initData->depthStencilInfo = depthStencilInfo;
	m_initData->descriptorSetLayouts = descriptorSetLayouts;
	m_initData->rasterizationState = rasterizationState;

	SetStageSourceFilePath(Anvil::ShaderStage::SHADER_STAGE_VERTEX,vsShader);
	SetStageSourceFilePath(Anvil::ShaderStage::FRAGMENT,fsShader);
	SetStageSourceFilePath(Anvil::ShaderStage::SHADER_STAGE_GEOMETRY,gsShader);
}*/
template<class TShaderBase>
	void ShaderLuaBase<TShaderBase>::InitializeRenderPass(std::shared_ptr<Anvil::RenderPass> &outRenderPass,Anvil::SubPassID &outSubPass,uint32_t pipelineIdx)
{
	//TShaderBase::InitializeRenderPass(outRenderPass,outSubPass,pipelineIdx);
	//CallLuaMember<void,uint32_t>("InitializeRenderPass",pipelineIdx); // TODO
}
template<class TShaderBase>
	void ShaderLuaBase<TShaderBase>::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	/*TShaderBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	m_currentPipelineInfo = &pipelineInfo;
	CallLuaMember<void,uint32_t>("InitializePipeline",pipelineIdx);
	m_currentPipelineInfo = nullptr;*/ // TODO
}
template<class TShaderBase>
	void ShaderLuaBase<TShaderBase>::AddDescriptorSetGroup(prosper::Shader::DescriptorSetInfo &descSetInfo)
{
	if(m_currentPipelineInfo == nullptr)
		return;
	//AddDescriptorSetGroup(*m_currentPipelineInfo,descSetInfo); // TODO
}
template<class TShaderBase>
	bool ShaderLuaBase<TShaderBase>::AttachPushConstantRange(uint32_t offset,uint32_t size,vk::ShaderStageFlags stages)
{
	if(m_currentPipelineInfo == nullptr)
		return false;
	return false; // TODO AttachPushConstantRange(*m_currentPipelineInfo,offset,size,stages);
}
/*template<class TShaderBase>
	void ShaderLuaBase<TShaderBase>::AddVertexAttribute(VertexAttribute &attr)
{
	if(m_currentPipelineInfo == nullptr)
		return;
	AddVertexAttribute(*m_currentPipelineInfo,attr);
}*/ // TODO

template<class TShaderBase>
	ShaderLuaGraphicsBase<TShaderBase>::ShaderLuaGraphicsBase(const luabind::object &o,prosper::Context &context,const std::string &identifier)
		: ShaderLuaBase<TShaderBase>(o,context,identifier)
{}

//////////////////

ShaderLua::ShaderLua()
	//: ShaderLuaBase<prosper::ShaderGraphics>({},*c_engine,"")
{}

ShaderLuaGraphics::ShaderLuaGraphics()
	//: ShaderLuaGraphicsBase<prosper::ShaderGraphics>({},*c_engine,"")
{}

ShaderLuaCompute::ShaderLuaCompute()
	//: ShaderLuaBase<prosper::ShaderCompute>({},*c_engine,"")
{}

//////////////////

ShaderLuaTextured::ShaderLuaTextured()
	//: ShaderLuaGraphicsBase<ShaderTextured3DBase>({},*c_engine,"")
{}

//////////////////

static void get_descriptor_set_layout_bindings(lua_State *l,std::vector<prosper::Shader::DescriptorSetInfo::Binding> &bindings,int32_t tBindings)
{
	auto numBindings = Lua::GetObjectLength(l,tBindings);
	bindings.reserve(numBindings);
	for(auto j=decltype(numBindings){0};j<numBindings;++j)
	{
		Lua::PushInt(l,j +1); /* 1 */
		Lua::GetTableValue(l,tBindings);

		Lua::CheckTable(l,-1);
		auto tBinding = Lua::GetStackTop(l);

		auto type = Anvil::DescriptorType::UNIFORM_BUFFER;
		Lua::get_table_value<ptrdiff_t,decltype(type)>(l,"type",tBinding,type,Lua::CheckInt);

		auto shaderStages = vk::ShaderStageFlagBits::eAllGraphics;
		Lua::get_table_value<ptrdiff_t,decltype(shaderStages)>(l,"stage",tBinding,shaderStages,Lua::CheckInt);

		uint32_t arrayCount = 1;
		Lua::get_table_value<ptrdiff_t,decltype(arrayCount)>(l,"arrayCount",tBinding,arrayCount,Lua::CheckInt);

		prosper::Shader::DescriptorSetInfo::Binding binding {type,shaderStages,arrayCount};
		bindings.push_back(binding);

		Lua::Pop(l,1); /* 0 */
	}
}

void LuaShaderManager::RegisterShader(std::string className,luabind::object &o)
{
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
	
	ustring::to_lower(className);
	m_shaders[className] = o;
	auto bOverwrite = true;

	auto oShader = luabind::object_cast_nothrow<prosper::Shader*>(r);
	auto *shader = oShader ? dynamic_cast<LuaShaderBase*>(oShader.get()) : nullptr;
	if(shader == nullptr)
	{
		Con::ccl<<"WARNING: Unable to create lua shader '"<<className<<"': Lua class is not derived from valid shader base!"<<Con::endl;
		return;
	}
	r.push(l); /* 1 */
	auto idx = Lua::GetStackTop(l);

	std::string fragmentShader;
	std::string vertexShader;
	std::string geometryShader;
	Lua::GetProtectedTableValue(l,idx,"FragmentShader",fragmentShader);
	Lua::GetProtectedTableValue(l,idx,"VertexShader",vertexShader);
	Lua::GetProtectedTableValue(l,idx,"GeometryShader",geometryShader);

	int32_t itopology;
	auto topology = vk::PrimitiveTopology::eTriangleList;
	if(Lua::GetProtectedTableValue(l,idx,"Topology",itopology) == Lua::StatusCode::Ok)
		topology = static_cast<vk::PrimitiveTopology>(itopology);

	// Dynamic States
	std::vector<vk::DynamicState> dynamicStates;
	if(Lua::GetProtectedTableValue(l,idx,"DynamicStates") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto t = Lua::GetStackTop(l);
			dynamicStates.reserve(Lua::GetObjectLength(l,t));
			Lua::PushNil(l); /* 2 */
			while(Lua::GetNextPair(l,t) != 0) /* 3 */
			{
				Lua::CheckNumber(l,-1);
				dynamicStates.push_back(static_cast<vk::DynamicState>(Lua::ToInt(l,-1)));
				Lua::Pop(l,1); /* 2 */
			} /* 1 */
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Rasterization State
	LuaVKPipelineRasterizationStateCreateInfo rasterizationState {};
	if(Lua::GetProtectedTableValue(l,idx,"RasterizationState") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto tState = Lua::GetStackTop(l);

			rasterizationState.custom[0] = Lua::get_table_value<ptrdiff_t,std::underlying_type_t<vk::CullModeFlagBits>>(l,"cullMode",tState,reinterpret_cast<std::underlying_type_t<vk::CullModeFlagBits>&>(rasterizationState.cullMode),Lua::CheckInt);
			rasterizationState.custom[1] = Lua::get_table_value<double,decltype(rasterizationState.depthBiasClamp)>(l,"depthBiasClamp",tState,rasterizationState.depthBiasClamp,Lua::CheckNumber);
			rasterizationState.custom[2] = Lua::get_table_value<double,decltype(rasterizationState.depthBiasConstantFactor)>(l,"depthBiasConstantFactor",tState,rasterizationState.depthBiasConstantFactor,Lua::CheckNumber);
			rasterizationState.custom[3] = Lua::get_table_value<bool,decltype(rasterizationState.depthBiasEnable)>(l,"depthBiasEnable",tState,rasterizationState.depthBiasEnable,Lua::CheckBool);
			rasterizationState.custom[4] = Lua::get_table_value<double,decltype(rasterizationState.depthBiasSlopeFactor)>(l,"depthBiasSlopeFactor",tState,rasterizationState.depthBiasSlopeFactor,Lua::CheckNumber);
			rasterizationState.custom[5] = Lua::get_table_value<bool,decltype(rasterizationState.depthClampEnable)>(l,"depthClampEnable",tState,rasterizationState.depthClampEnable,Lua::CheckBool);
			rasterizationState.custom[6] = Lua::get_table_value<ptrdiff_t,decltype(rasterizationState.frontFace)>(l,"frontFace",tState,rasterizationState.frontFace,Lua::CheckInt);
			rasterizationState.custom[7] = Lua::get_table_value<double,decltype(rasterizationState.lineWidth)>(l,"lineWidth",tState,rasterizationState.lineWidth,Lua::CheckNumber);
			rasterizationState.custom[8] = Lua::get_table_value<ptrdiff_t,decltype(rasterizationState.polygonMode)>(l,"polygonMode",tState,rasterizationState.polygonMode,Lua::CheckInt);
			rasterizationState.custom[9] = Lua::get_table_value<bool,decltype(rasterizationState.rasterizerDiscardEnable)>(l,"rasterizerDiscardEnable",tState,rasterizationState.rasterizerDiscardEnable,Lua::CheckBool);
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Push Constant Ranges
	std::vector<vk::PushConstantRange> pushConstantRanges;
	if(Lua::GetProtectedTableValue(l,idx,"PushConstantRanges") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto t = Lua::GetStackTop(l);
			auto num = Lua::GetObjectLength(l,t);
			pushConstantRanges.reserve(num);

			for(auto i=decltype(num){0};i<num;++i) // Order is important!
			{
				Lua::PushInt(l,i +1); /* 2 */
				Lua::GetTableValue(l,t);

				Lua::CheckTable(l,-1);

				// Push Constant Range
				auto t2 = Lua::GetStackTop(l);
				auto stage = vk::ShaderStageFlagBits::eAllGraphics;
				Lua::PushString(l,"stage"); /* 3 */
				Lua::GetTableValue(l,t2);
				if(Lua::IsSet(l,-1) == true)
					stage = static_cast<vk::ShaderStageFlagBits>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1); /* 2 */

				uint32_t offset = 0;
				Lua::PushString(l,"offset"); /* 3 */
				Lua::GetTableValue(l,t2);
				if(Lua::IsSet(l,-1) == true)
					offset = static_cast<uint32_t>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1); /* 2 */

				uint32_t size = 0;
				Lua::PushString(l,"size"); /* 3 */
				Lua::GetTableValue(l,t2);
				if(Lua::IsSet(l,-1) == true)
					size = static_cast<uint32_t>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1); /* 2 */

				pushConstantRanges.push_back({stage,offset,size});
				//

				Lua::Pop(l,1); /* 1 */
			}
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Vertex Input
	std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
	std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
	if(Lua::GetProtectedTableValue(l,idx,"VertexInput") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			auto tVertexInput = Lua::GetStackTop(l);
			Lua::CheckTable(l,-1);

			if(Lua::GetProtectedTableValue(l,tVertexInput,"BindingDescriptions") == Lua::StatusCode::Ok) /* 2 */
			{
				if(!Lua::IsNil(l,-1))
				{
					auto tBindingDescriptions = Lua::GetStackTop(l);
					Lua::CheckTable(l,-1);
					auto num = Lua::GetObjectLength(l,tBindingDescriptions);
					vertexBindingDescriptions.reserve(num);

					for(auto i=decltype(num){0};i<num;++i) // Order is important!
					{
						Lua::PushInt(l,i +1); /* 3 */
						Lua::GetTableValue(l,tBindingDescriptions);

						Lua::CheckTable(l,-1);
						auto tDescription = Lua::GetStackTop(l);
						vertexBindingDescriptions.push_back({});
						auto &desc = vertexBindingDescriptions.back();

						Lua::PushString(l,"binding"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.binding = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"inputRate"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.inputRate = static_cast<vk::VertexInputRate>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"stride"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.stride = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::Pop(l,1); /* 2 */
					}
				}
				Lua::Pop(l,1); /* 1 */
			}


			if(Lua::GetProtectedTableValue(l,tVertexInput,"AttributeDescriptions") == Lua::StatusCode::Ok) /* 2 */
			{
				if(!Lua::IsNil(l,-1))
				{
					auto tAttributeDescriptions = Lua::GetStackTop(l);
					Lua::CheckTable(l,-1);
					auto num = Lua::GetObjectLength(l,tAttributeDescriptions);
					vertexAttributeDescriptions.reserve(num);

					for(auto i=decltype(num){0};i<num;++i) // Order is important!
					{
						Lua::PushInt(l,i +1); /* 3 */
						Lua::GetTableValue(l,tAttributeDescriptions);

						Lua::CheckTable(l,-1);
						auto tDescription = Lua::GetStackTop(l);
						vertexAttributeDescriptions.push_back({});
						auto &desc = vertexAttributeDescriptions.back();

						Lua::PushString(l,"binding"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.binding = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"format"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.format = static_cast<vk::Format>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"location"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.location = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"offset"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.offset = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::Pop(l,1); /* 2 */
					}
				}
				Lua::Pop(l,1); /* 1 */
			}
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Depth Stencil State
	vk::PipelineDepthStencilStateCreateInfo depthStencilInfo {};
	depthStencilInfo.setDepthCompareOp(Anvil::CompareOp::LESS_OR_EQUAL);
	if(Lua::GetProtectedTableValue(l,idx,"DepthStencilState") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto tState = Lua::GetStackTop(l);

			Lua::get_table_value<bool,decltype(depthStencilInfo.depthBoundsTestEnable)>(l,"depthBoundsTestEnable",tState,depthStencilInfo.depthBoundsTestEnable,Lua::CheckBool);
			Lua::get_table_value<ptrdiff_t,decltype(depthStencilInfo.depthCompareOp)>(l,"depthCompareOp",tState,depthStencilInfo.depthCompareOp,Lua::CheckInt);
			Lua::get_table_value<bool,decltype(depthStencilInfo.depthTestEnable)>(l,"depthTestEnable",tState,depthStencilInfo.depthTestEnable,Lua::CheckBool);
			Lua::get_table_value<bool,decltype(depthStencilInfo.depthWriteEnable)>(l,"depthWriteEnable",tState,depthStencilInfo.depthWriteEnable,Lua::CheckBool);
			Lua::get_table_value<double,decltype(depthStencilInfo.maxDepthBounds)>(l,"maxDepthBounds",tState,depthStencilInfo.maxDepthBounds,Lua::CheckNumber);
			Lua::get_table_value<double,decltype(depthStencilInfo.minDepthBounds)>(l,"minDepthBounds",tState,depthStencilInfo.minDepthBounds,Lua::CheckNumber);
			std::array<vk::StencilOpState*,2> stencilStates = {&depthStencilInfo.front,&depthStencilInfo.back};
			for(auto i=decltype(stencilStates.size()){0};i<stencilStates.size();++i)
			{
				if(Lua::GetProtectedTableValue(l,tState,(i == 0) ? "front" : "back") == Lua::StatusCode::Ok) /* 2 */
				{
					if(!Lua::IsNil(l,-1))
					{
						//auto tFront = Lua::GetStackTop(l);
						Lua::CheckTable(l,-1);

						auto &state = *stencilStates[i];
						Lua::get_table_value<ptrdiff_t,decltype(state.compareMask)>(l,"compareMask",tState,state.compareMask,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.compareOp)>(l,"compareOp",tState,state.compareOp,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.depthFailOp)>(l,"depthFailOp",tState,state.depthFailOp,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.failOp)>(l,"failOp",tState,state.failOp,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.passOp)>(l,"passOp",tState,state.passOp,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.reference)>(l,"reference",tState,state.reference,Lua::CheckInt);
					}
					Lua::Pop(l,1); /* 1 */
				}
			}
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Descriptor Sets
	std::vector<prosper::Shader::DescriptorSetInfo> descSetLayouts;
	if(Lua::GetProtectedTableValue(l,idx,"DescriptorSetLayouts") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto t = Lua::GetStackTop(l);
			auto num = Lua::GetObjectLength(l,t);
			descSetLayouts.reserve(num);
			for(auto i=decltype(num){0};i<num;++i) // Order is important!
			{
				Lua::PushInt(l,i +1); /* 2 */
				Lua::GetTableValue(l,t);

				Lua::CheckTable(l,-1);
				auto tLayout = Lua::GetStackTop(l);
				descSetLayouts.push_back({});
				auto &layout = descSetLayouts.back();
				auto &bindings = layout.bindings;
				if(Lua::GetProtectedTableValue(l,tLayout,"Bindings") == Lua::StatusCode::Ok) /* 3 */
				{
					if(!Lua::IsNil(l,-1))
					{
						Lua::CheckTable(l,-1);
						auto tBindings = Lua::GetStackTop(l);
						get_descriptor_set_layout_bindings(l,bindings,tBindings);
					}
					Lua::Pop(l,1); /* 2 */
				}
				Lua::Pop(l,1); /* 1 */
			}
		}
		Lua::Pop(l,1); /* 0 */
	}
	Lua::Pop(l,1); /* 0 */

	auto *baseShader = oShader.get();
	/*shader->Initialize(
		r,topology,dynamicStates,pushConstantRanges,vertexBindingDescriptions,vertexAttributeDescriptions,
		descSetLayouts,rasterizationState,depthStencilInfo,vertexShader,fragmentShader,geometryShader
	);*/ // TODO
	c_engine->GetShaderManager().RegisterShader(className,[baseShader](prosper::Context &context,const std::string &identifier,bool &bExternalOwnership) {
		bExternalOwnership = true;
		return baseShader;
	});
}

luabind::object *LuaShaderManager::GetClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_shaders.find(className);
	if(it == m_shaders.end())
		return nullptr;
	return &it->second;
}
#endif

 // prosper TODO
#if 0
#include "c_shader_lua.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <pragma/lua/luaapi.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

Shader::Lua::LuaBase::LuaBase(const luabind::object &o,std::string &identifier,std::vector<std::string> &sources)
	: LuaObjectBase(o),m_identifier(identifier),m_shaderSources(sources)
{}
void Shader::Lua::LuaBase::ClearLuaObject()
{
	m_baseLuaObj = nullptr;
}
void Shader::Lua::LuaBase::InitializePipelineLayout(const Vulkan::Context&,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	setLayouts.reserve(setLayouts.size() +m_descriptorSetLayouts.size());
	for(auto &setLayout : m_descriptorSetLayouts)
		setLayouts.push_back(setLayout);

	pushConstants.reserve(pushConstants.size() +m_pushConstantRanges.size());
	for(auto &range : m_pushConstantRanges)
		pushConstants.push_back(range);
}
void Shader::Lua::LuaBase::InitializeDynamicStates(std::vector<vk::DynamicState> &states)
{
	states.reserve(states.size() +m_dynamicStates.size());
	for(auto state : m_dynamicStates)
		states.push_back(state);
}
void Shader::Lua::LuaBase::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	vertexBindingDescriptions.reserve(vertexBindingDescriptions.size() +m_vertexBindingDescriptions.size());
	vertexAttributeDescriptions.reserve(vertexAttributeDescriptions.size() +m_vertexAttributeDescriptions.size());
	for(auto &desc : m_vertexBindingDescriptions)
		vertexBindingDescriptions.push_back(desc);
	for(auto &desc : m_vertexAttributeDescriptions)
		vertexAttributeDescriptions.push_back(desc);
}
void Shader::Lua::LuaBase::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	const_cast<vk::PipelineDepthStencilStateCreateInfo&>(*info.pDepthStencilState) = m_depthStencilInfo;
	auto *assemblyState = const_cast<vk::PipelineInputAssemblyStateCreateInfo*>(info.pInputAssemblyState);
	assemblyState->topology = m_topology;
	auto *rasterizationState = const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState);

	if(m_rasterizationState.custom[0] == true) rasterizationState->cullMode = m_rasterizationState.cullMode;
	if(m_rasterizationState.custom[1] == true) rasterizationState->depthBiasClamp = m_rasterizationState.depthBiasClamp;
	if(m_rasterizationState.custom[2] == true) rasterizationState->depthBiasConstantFactor = m_rasterizationState.depthBiasConstantFactor;
	if(m_rasterizationState.custom[3] == true) rasterizationState->depthBiasEnable = m_rasterizationState.depthBiasEnable;
	if(m_rasterizationState.custom[4] == true) rasterizationState->depthBiasSlopeFactor = m_rasterizationState.depthBiasSlopeFactor;
	if(m_rasterizationState.custom[5] == true) rasterizationState->depthClampEnable = m_rasterizationState.depthClampEnable;
	if(m_rasterizationState.custom[6] == true) rasterizationState->frontFace = m_rasterizationState.frontFace;
	if(m_rasterizationState.custom[7] == true) rasterizationState->lineWidth = m_rasterizationState.lineWidth;
	if(m_rasterizationState.custom[8] == true) rasterizationState->polygonMode = m_rasterizationState.polygonMode;
	if(m_rasterizationState.custom[9] == true) rasterizationState->rasterizerDiscardEnable = m_rasterizationState.rasterizerDiscardEnable;
}
void Shader::Lua::LuaBase::Initialize(
	const luabind::object &o,vk::PrimitiveTopology topology,const std::vector<vk::DynamicState> &dynamicStates,const std::vector<Vulkan::PushConstantRange> &pushConstantRanges,
	const std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,const std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,const std::vector<Vulkan::DescriptorSetLayout> &descriptorSetLayouts,
	const ::Lua::VKPipelineRasterizationStateCreateInfo &rasterizationState,const vk::PipelineDepthStencilStateCreateInfo &depthStencilInfo,
	const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
{
	m_baseLuaObj = std::shared_ptr<luabind::object>(new luabind::object(o));
	m_topology = topology;
	m_dynamicStates = dynamicStates;
	m_pushConstantRanges = pushConstantRanges;
	m_vertexBindingDescriptions = vertexBindingDescriptions;
	m_vertexAttributeDescriptions = vertexAttributeDescriptions;
	m_depthStencilInfo = depthStencilInfo;
	m_descriptorSetLayouts = descriptorSetLayouts;
	m_rasterizationState = rasterizationState;

	m_identifier = identifier;
	m_shaderSources.clear();
	m_shaderSources.push_back(vsShader);
	m_shaderSources.push_back(fsShader);
	m_shaderSources.push_back(gsShader);
}

////////////////////////////////

Shader::Lua::Base::Base()
	: ::Shader::Base("","",""),Lua::LuaBase({},::Shader::Base::m_identifier,::Shader::Base::m_shaderSources)
{}
void Shader::Lua::Base::InitializeShaderPipelines(const Vulkan::Context &context)
{
	::Shader::Base::InitializeShaderPipelines(context);
	CallLuaMember<void>("InitializePipeline");
}
void Shader::Lua::Base::InitializeDynamicStates(std::vector<vk::DynamicState> &states)
{
	::Shader::Base::InitializeDynamicStates(states);
	Lua::LuaBase::InitializeDynamicStates(states);
}
void Shader::Lua::Base::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Lua::LuaBase::InitializePipelineLayout(context,setLayouts,pushConstants);
	::Shader::Base::InitializePipelineLayout(context,setLayouts,pushConstants); // Needs to be called AFTER LuaBase!
}
void Shader::Lua::Base::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	::Shader::Base::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	Lua::LuaBase::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
}
void Shader::Lua::Base::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	Lua::LuaBase::SetupPipeline(pipelineIdx,info);
	::Shader::Base::SetupPipeline(pipelineIdx,info); // Needs to be called AFTER LuaBase!
}

////////////////////////////////

Shader::Lua::Textured::Textured()
	: Textured3D("","",""),Lua::LuaBase({},::Shader::Base::m_identifier,::Shader::Base::m_shaderSources)
{}
void Shader::Lua::Textured::InitializeShader() {TexturedBase3D::InitializeShader();}
void Shader::Lua::Textured::InitializeShaderPipelines(const Vulkan::Context &context)
{
	Textured3D::InitializeShaderPipelines(context);
	CallLuaMember<void>("InitializePipeline");
}
void Shader::Lua::Textured::InitializeDynamicStates(std::vector<vk::DynamicState> &states)
{
	Textured3D::InitializeDynamicStates(states);
	Lua::LuaBase::InitializeDynamicStates(states);
}
void Shader::Lua::Textured::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	std::vector<Vulkan::DescriptorSetLayout> luaSetLayouts;
	std::vector<Vulkan::PushConstantRange> luaPushConstants;
	Lua::LuaBase::InitializePipelineLayout(context,luaSetLayouts,luaPushConstants);
	Textured3D::InitializePipelineLayout(context,setLayouts,pushConstants); // Needs to be called AFTER LuaBase!
	setLayouts.reserve(setLayouts.size() +luaSetLayouts.size());
	pushConstants.reserve(pushConstants.size() +luaPushConstants.size());
	for(auto &layout : luaSetLayouts)
		setLayouts.push_back(layout);
	for(auto &pc : luaPushConstants)
		pushConstants.push_back(pc);
}
void Shader::Lua::Textured::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Textured3D::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	Lua::LuaBase::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
}
void Shader::Lua::Textured::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	Lua::LuaBase::SetupPipeline(pipelineIdx,info);
	Textured3D::SetupPipeline(pipelineIdx,info); // Needs to be called AFTER LuaBase!
}
void Shader::Lua::Textured::BindMaterial(Material *mat)
{
	Textured3D::BindMaterial(mat);
	BindMaterialParameters(mat);
}
bool Shader::Lua::Textured::BindEntity(Vulkan::CommandBufferObject *cmdBuffer,CBaseEntity *ent,bool &bWeighted)
{
	auto r = Textured3D::BindEntity(cmdBuffer,ent,bWeighted);
	if(r == false)
		return r;
	CallLuaMember<void,::Lua::Vulkan::CommandBufferHandleOwner,luabind::object>("OnBindEntity",::Lua::Vulkan::CommandBufferHandleOwner(cmdBuffer->GetHandle(),cmdBuffer->ClaimOwnership()),*ent->GetLuaObject());
	return r;
}
void Shader::Lua::Textured::BindScene(Vulkan::CommandBufferObject *cmdBuffer,const Scene &scene,bool bView)
{
	Textured3D::BindScene(cmdBuffer,scene,bView);
	CallLuaMember<void,::Lua::Vulkan::CommandBufferHandleOwner,std::shared_ptr<::Scene>>("OnBindScene",::Lua::Vulkan::CommandBufferHandleOwner(cmdBuffer->GetHandle(),cmdBuffer->ClaimOwnership()),const_cast<Scene&>(scene).shared_from_this());
}
void Shader::Lua::Textured::OnBindEntity(::Lua::Vulkan::CommandBufferHandleOwner &hCmdBuffer,EntityHandle &hEnt) {}
void Shader::Lua::Textured::OnBindScene(::Lua::Vulkan::CommandBufferHandleOwner &hCmdBuffer,std::shared_ptr<Scene> &scene) {}
void Shader::Lua::Textured::default_OnBindEntity(lua_State *l,::Lua::Vulkan::CommandBufferHandleOwner &hCmdBuffer,EntityHandle &hEnt) {}
void Shader::Lua::Textured::default_OnBindScene(lua_State *l,::Lua::Vulkan::CommandBufferHandleOwner &hCmdBuffer,std::shared_ptr<Scene> &scene) {}
void Shader::Lua::Textured::ImplDraw(CModelSubMesh *mesh)
{
	Textured3D::Draw(mesh);
}
void Shader::Lua::Textured::ImplInitializeMaterial(Material *mat,bool bReload)
{
	Textured3D::InitializeMaterial(mat,bReload);
}
void Shader::Lua::Textured::InitializeMaterial(Material *mat,bool bReload)
{
	if(m_baseLuaObj == nullptr)
		return;
	CallLuaMember<void,Material*>("InitializeMaterial",mat);
}
void Shader::Lua::Textured::default_InitializeMaterial(lua_State*,Textured *shader,Material *mat,bool bReload)
{
	shader->ImplInitializeMaterial(mat,bReload);
}
void Shader::Lua::Textured::Draw(CModelSubMesh *mesh)
{
	if(m_baseLuaObj == nullptr)
		return;
	CallLuaMember<void,std::shared_ptr<ModelSubMesh>>("Draw",mesh->shared_from_this());
}
void Shader::Lua::Textured::Draw(std::shared_ptr<ModelSubMesh>&) {}
void Shader::Lua::Textured::default_Draw(lua_State*,Textured *shader,std::shared_ptr<ModelSubMesh> &mesh)
{
	shader->ImplDraw(static_cast<CModelSubMesh*>(mesh.get()));
}

//////////////////////////////

void LuaShaderManager::RegisterShader(std::string className,luabind::object &o)
{
	ustring::to_lower(className);
	m_shaders[className] = o;
	auto bOverwrite = true;

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
	
	auto oShader = luabind::object_cast_nothrow<Shader::Base*>(r);
	auto *shader = oShader ? dynamic_cast<Shader::Lua::LuaBase*>(oShader.get()) : nullptr;
	if(shader == nullptr)
	{
		Con::ccl<<"WARNING: Unable to create lua shader '"<<className<<"': Lua class is not derived from valid shader base!"<<Con::endl;
		return;
	}
	r.push(l); /* 1 */
	auto idx = Lua::GetStackTop(l);

	std::string fragmentShader;
	std::string vertexShader;
	std::string geometryShader;
	Lua::GetProtectedTableValue(l,idx,"FragmentShader",fragmentShader);
	Lua::GetProtectedTableValue(l,idx,"VertexShader",vertexShader);
	Lua::GetProtectedTableValue(l,idx,"GeometryShader",geometryShader);

	int32_t itopology;
	auto topology = vk::PrimitiveTopology::eTriangleList;
	if(Lua::GetProtectedTableValue(l,idx,"Topology",itopology) == Lua::StatusCode::Ok)
		topology = static_cast<vk::PrimitiveTopology>(itopology);

	auto bBlendEnable = false;
	auto bHasCustomBlend = (Lua::GetProtectedTableValue(l,idx,"BlendEnable",bBlendEnable) == Lua::StatusCode::Ok) ? true : false;

	// Dynamic States
	std::vector<vk::DynamicState> dynamicStates;
	if(Lua::GetProtectedTableValue(l,idx,"DynamicStates") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto t = Lua::GetStackTop(l);
			dynamicStates.reserve(Lua::GetObjectLength(l,t));
			Lua::PushNil(l); /* 2 */
			while(Lua::GetNextPair(l,t) != 0) /* 3 */
			{
				Lua::CheckNumber(l,-1);
				dynamicStates.push_back(static_cast<vk::DynamicState>(Lua::ToInt(l,-1)));
				Lua::Pop(l,1); /* 2 */
			} /* 1 */
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Rasterization State
	Lua::VKPipelineRasterizationStateCreateInfo rasterizationState {};
	if(Lua::GetProtectedTableValue(l,idx,"RasterizationState") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto tState = Lua::GetStackTop(l);

			rasterizationState.custom[0] = Lua::get_table_value<ptrdiff_t,std::underlying_type_t<vk::CullModeFlagBits>>(l,"cullMode",tState,reinterpret_cast<std::underlying_type_t<vk::CullModeFlagBits>&>(rasterizationState.cullMode),Lua::CheckInt);
			rasterizationState.custom[1] = Lua::get_table_value<double,decltype(rasterizationState.depthBiasClamp)>(l,"depthBiasClamp",tState,rasterizationState.depthBiasClamp,Lua::CheckNumber);
			rasterizationState.custom[2] = Lua::get_table_value<double,decltype(rasterizationState.depthBiasConstantFactor)>(l,"depthBiasConstantFactor",tState,rasterizationState.depthBiasConstantFactor,Lua::CheckNumber);
			rasterizationState.custom[3] = Lua::get_table_value<bool,decltype(rasterizationState.depthBiasEnable)>(l,"depthBiasEnable",tState,rasterizationState.depthBiasEnable,Lua::CheckBool);
			rasterizationState.custom[4] = Lua::get_table_value<double,decltype(rasterizationState.depthBiasSlopeFactor)>(l,"depthBiasSlopeFactor",tState,rasterizationState.depthBiasSlopeFactor,Lua::CheckNumber);
			rasterizationState.custom[5] = Lua::get_table_value<bool,decltype(rasterizationState.depthClampEnable)>(l,"depthClampEnable",tState,rasterizationState.depthClampEnable,Lua::CheckBool);
			rasterizationState.custom[6] = Lua::get_table_value<ptrdiff_t,decltype(rasterizationState.frontFace)>(l,"frontFace",tState,rasterizationState.frontFace,Lua::CheckInt);
			rasterizationState.custom[7] = Lua::get_table_value<double,decltype(rasterizationState.lineWidth)>(l,"lineWidth",tState,rasterizationState.lineWidth,Lua::CheckNumber);
			rasterizationState.custom[8] = Lua::get_table_value<ptrdiff_t,decltype(rasterizationState.polygonMode)>(l,"polygonMode",tState,rasterizationState.polygonMode,Lua::CheckInt);
			rasterizationState.custom[9] = Lua::get_table_value<bool,decltype(rasterizationState.rasterizerDiscardEnable)>(l,"rasterizerDiscardEnable",tState,rasterizationState.rasterizerDiscardEnable,Lua::CheckBool);
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Push Constant Ranges
	std::vector<Vulkan::PushConstantRange> pushConstantRanges;
	if(Lua::GetProtectedTableValue(l,idx,"PushConstantRanges") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto t = Lua::GetStackTop(l);
			auto num = Lua::GetObjectLength(l,t);
			pushConstantRanges.reserve(num);

			for(auto i=decltype(num){0};i<num;++i) // Order is important!
			{
				Lua::PushInt(l,i +1); /* 2 */
				Lua::GetTableValue(l,t);

				Lua::CheckTable(l,-1);

				// Push Constant Range
				auto t2 = Lua::GetStackTop(l);
				auto stage = vk::ShaderStageFlagBits::eAllGraphics;
				Lua::PushString(l,"stage"); /* 3 */
				Lua::GetTableValue(l,t2);
				if(Lua::IsSet(l,-1) == true)
					stage = static_cast<vk::ShaderStageFlagBits>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1); /* 2 */

				uint32_t offset = 0;
				Lua::PushString(l,"offset"); /* 3 */
				Lua::GetTableValue(l,t2);
				if(Lua::IsSet(l,-1) == true)
					offset = static_cast<uint32_t>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1); /* 2 */

				uint32_t size = 0;
				Lua::PushString(l,"size"); /* 3 */
				Lua::GetTableValue(l,t2);
				if(Lua::IsSet(l,-1) == true)
					size = static_cast<uint32_t>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1); /* 2 */

				pushConstantRanges.push_back({stage,offset /sizeof(uint32_t),size /sizeof(uint32_t)});
				//

				Lua::Pop(l,1); /* 1 */
			}
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Vertex Input
	std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
	std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
	if(Lua::GetProtectedTableValue(l,idx,"VertexInput") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			auto tVertexInput = Lua::GetStackTop(l);
			Lua::CheckTable(l,-1);

			if(Lua::GetProtectedTableValue(l,tVertexInput,"BindingDescriptions") == Lua::StatusCode::Ok) /* 2 */
			{
				if(!Lua::IsNil(l,-1))
				{
					auto tBindingDescriptions = Lua::GetStackTop(l);
					Lua::CheckTable(l,-1);
					auto num = Lua::GetObjectLength(l,tBindingDescriptions);
					vertexBindingDescriptions.reserve(num);

					for(auto i=decltype(num){0};i<num;++i) // Order is important!
					{
						Lua::PushInt(l,i +1); /* 3 */
						Lua::GetTableValue(l,tBindingDescriptions);

						Lua::CheckTable(l,-1);
						auto tDescription = Lua::GetStackTop(l);
						vertexBindingDescriptions.push_back({});
						auto &desc = vertexBindingDescriptions.back();

						Lua::PushString(l,"binding"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.binding = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"inputRate"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.inputRate = static_cast<vk::VertexInputRate>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"stride"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.stride = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::Pop(l,1); /* 2 */
					}
				}
				Lua::Pop(l,1); /* 1 */
			}


			if(Lua::GetProtectedTableValue(l,tVertexInput,"AttributeDescriptions") == Lua::StatusCode::Ok) /* 2 */
			{
				if(!Lua::IsNil(l,-1))
				{
					auto tAttributeDescriptions = Lua::GetStackTop(l);
					Lua::CheckTable(l,-1);
					auto num = Lua::GetObjectLength(l,tAttributeDescriptions);
					vertexAttributeDescriptions.reserve(num);

					for(auto i=decltype(num){0};i<num;++i) // Order is important!
					{
						Lua::PushInt(l,i +1); /* 3 */
						Lua::GetTableValue(l,tAttributeDescriptions);

						Lua::CheckTable(l,-1);
						auto tDescription = Lua::GetStackTop(l);
						vertexAttributeDescriptions.push_back({});
						auto &desc = vertexAttributeDescriptions.back();

						Lua::PushString(l,"binding"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.binding = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"format"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.format = static_cast<vk::Format>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"location"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.location = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::PushString(l,"offset"); /* 4 */
						Lua::GetTableValue(l,tDescription);
						if(Lua::IsSet(l,-1) == true)
							desc.offset = static_cast<uint32_t>(Lua::CheckInt(l,-1));
						Lua::Pop(l,1); /* 3 */

						Lua::Pop(l,1); /* 2 */
					}
				}
				Lua::Pop(l,1); /* 1 */
			}
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Depth Stencil State
	vk::PipelineDepthStencilStateCreateInfo depthStencilInfo {};
	depthStencilInfo.setDepthCompareOp(Anvil::CompareOp::LESS_OR_EQUAL);
	if(Lua::GetProtectedTableValue(l,idx,"DepthStencilState") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto tState = Lua::GetStackTop(l);

			Lua::get_table_value<bool,decltype(depthStencilInfo.depthBoundsTestEnable)>(l,"depthBoundsTestEnable",tState,depthStencilInfo.depthBoundsTestEnable,Lua::CheckBool);
			Lua::get_table_value<ptrdiff_t,decltype(depthStencilInfo.depthCompareOp)>(l,"depthCompareOp",tState,depthStencilInfo.depthCompareOp,Lua::CheckInt);
			Lua::get_table_value<bool,decltype(depthStencilInfo.depthTestEnable)>(l,"depthTestEnable",tState,depthStencilInfo.depthTestEnable,Lua::CheckBool);
			Lua::get_table_value<bool,decltype(depthStencilInfo.depthWriteEnable)>(l,"depthWriteEnable",tState,depthStencilInfo.depthWriteEnable,Lua::CheckBool);
			Lua::get_table_value<double,decltype(depthStencilInfo.maxDepthBounds)>(l,"maxDepthBounds",tState,depthStencilInfo.maxDepthBounds,Lua::CheckNumber);
			Lua::get_table_value<double,decltype(depthStencilInfo.minDepthBounds)>(l,"minDepthBounds",tState,depthStencilInfo.minDepthBounds,Lua::CheckNumber);
			std::array<vk::StencilOpState*,2> stencilStates = {&depthStencilInfo.front,&depthStencilInfo.back};
			for(auto i=decltype(stencilStates.size()){0};i<stencilStates.size();++i)
			{
				if(Lua::GetProtectedTableValue(l,tState,(i == 0) ? "front" : "back") == Lua::StatusCode::Ok) /* 2 */
				{
					if(!Lua::IsNil(l,-1))
					{
						//auto tFront = Lua::GetStackTop(l);
						Lua::CheckTable(l,-1);

						auto &state = *stencilStates[i];
						Lua::get_table_value<ptrdiff_t,decltype(state.compareMask)>(l,"compareMask",tState,state.compareMask,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.compareOp)>(l,"compareOp",tState,state.compareOp,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.depthFailOp)>(l,"depthFailOp",tState,state.depthFailOp,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.failOp)>(l,"failOp",tState,state.failOp,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.passOp)>(l,"passOp",tState,state.passOp,Lua::CheckInt);
						Lua::get_table_value<ptrdiff_t,decltype(state.reference)>(l,"reference",tState,state.reference,Lua::CheckInt);
					}
					Lua::Pop(l,1); /* 1 */
				}
			}
		}
		Lua::Pop(l,1); /* 0 */
	}

	// Descriptor Sets
	std::vector<Vulkan::DescriptorSetLayout> descSetLayouts;
	if(Lua::GetProtectedTableValue(l,idx,"DescriptorSetLayouts") == Lua::StatusCode::Ok) /* 1 */
	{
		if(!Lua::IsNil(l,-1))
		{
			Lua::CheckTable(l,-1);
			auto t = Lua::GetStackTop(l);
			auto num = Lua::GetObjectLength(l,t);
			descSetLayouts.reserve(num);
			for(auto i=decltype(num){0};i<num;++i) // Order is important!
			{
				Lua::PushInt(l,i +1); /* 2 */
				Lua::GetTableValue(l,t);

				Lua::CheckTable(l,-1);
				auto tLayout = Lua::GetStackTop(l);
				std::vector<Vulkan::DescriptorSetLayout::Binding> bindings;
				if(Lua::GetProtectedTableValue(l,tLayout,"Bindings") == Lua::StatusCode::Ok) /* 3 */
				{
					if(!Lua::IsNil(l,-1))
					{
						Lua::CheckTable(l,-1);
						auto tBindings = Lua::GetStackTop(l);
						Lua::Vulkan::get_descriptor_set_layout_bindings(l,bindings,tBindings);
					}
					Lua::Pop(l,1); /* 2 */
				}
				descSetLayouts.push_back(Vulkan::DescriptorSetLayout::Create(c_engine->GetRenderContext(),bindings));
				Lua::Pop(l,1); /* 1 */
			}
		}
		Lua::Pop(l,1); /* 0 */
	}
	Lua::Pop(l,1); /* 0 */

	auto *baseShader = oShader.get();
	if(bHasCustomBlend == true)
		baseShader->SetUseAlpha(bBlendEnable);
	shader->Initialize(r,topology,dynamicStates,pushConstantRanges,vertexBindingDescriptions,vertexAttributeDescriptions,descSetLayouts,rasterizationState,depthStencilInfo,className,vertexShader,fragmentShader,geometryShader);

	try
	{
		ShaderSystem::register_shader(c_engine->GetRenderContext(),className,baseShader->GetHandle(),bOverwrite,nullptr);
	}
	catch(Vulkan::Exception &e)
	{
		Con::ccl<<"WARNING: Unable to create lua shader '"<<className<<"': "<<e.what()<<" ("<<e.GetErrorCode()<<")"<<"!"<<Con::endl;
	}
	///shader->Initialize(c_engine->GetRenderContext(),logCallback);
	//c_engine->LoadShader(className,true);
}

luabind::object *LuaShaderManager::GetClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_shaders.find(className);
	if(it == m_shaders.end())
		return nullptr;
	return &it->second;
}
#endif


