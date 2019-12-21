#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <pragma/game/game_limits.h>
#include <datasystem_color.h>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;


#pragma optimize("",off)
ShaderTextured3DBase::Pipeline ShaderTextured3DBase::GetPipelineIndex(Anvil::SampleCountFlagBits sampleCount,bool bReflection)
{
	if(sampleCount == Anvil::SampleCountFlagBits::_1_BIT)
		return bReflection ? Pipeline::Reflection : Pipeline::Regular;
	if(bReflection)
		throw std::logic_error("Multi-sampled reflection pipeline not supported!");
	return Pipeline::MultiSample;
}

decltype(ShaderTextured3DBase::HASH_TYPE) ShaderTextured3DBase::HASH_TYPE = typeid(ShaderTextured3DBase).hash_code();
decltype(ShaderTextured3DBase::VERTEX_BINDING_BONE_WEIGHT) ShaderTextured3DBase::VERTEX_BINDING_BONE_WEIGHT = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID,VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT,VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderTextured3DBase::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderTextured3DBase::VERTEX_BINDING_BONE_WEIGHT_EXT = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID,VERTEX_BINDING_BONE_WEIGHT_EXT};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT,VERTEX_BINDING_BONE_WEIGHT_EXT};

decltype(ShaderTextured3DBase::VERTEX_BINDING_VERTEX) ShaderTextured3DBase::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX,sizeof(VertexBufferData)};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_POSITION) ShaderTextured3DBase::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_UV) ShaderTextured3DBase::VERTEX_ATTRIBUTE_UV = {ShaderEntity::VERTEX_ATTRIBUTE_UV,VERTEX_BINDING_VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_NORMAL) ShaderTextured3DBase::VERTEX_ATTRIBUTE_NORMAL = {ShaderEntity::VERTEX_ATTRIBUTE_NORMAL,VERTEX_BINDING_VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_TANGENT) ShaderTextured3DBase::VERTEX_ATTRIBUTE_TANGENT = {ShaderEntity::VERTEX_ATTRIBUTE_TANGENT,VERTEX_BINDING_VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BI_TANGENT) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BI_TANGENT = {ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT,VERTEX_BINDING_VERTEX};

decltype(ShaderTextured3DBase::VERTEX_BINDING_LIGHTMAP) ShaderTextured3DBase::VERTEX_BINDING_LIGHTMAP = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_LIGHTMAP_UV) ShaderTextured3DBase::VERTEX_ATTRIBUTE_LIGHTMAP_UV = {ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV,VERTEX_BINDING_LIGHTMAP};

decltype(ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE) ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_MATERIAL) ShaderTextured3DBase::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Material settings
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::VERTEX_BIT | Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::GEOMETRY_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Diffuse Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Normal Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Specular Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Parallax Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Glow Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA) ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA = {&ShaderEntity::DESCRIPTOR_SET_CAMERA};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderTextured3DBase::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderEntity::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_LIGHTS) ShaderTextured3DBase::DESCRIPTOR_SET_LIGHTS = {&ShaderEntity::DESCRIPTOR_SET_LIGHTS};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_CSM) ShaderTextured3DBase::DESCRIPTOR_SET_CSM = {&ShaderEntity::DESCRIPTOR_SET_CSM};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_SHADOWS) ShaderTextured3DBase::DESCRIPTOR_SET_SHADOWS = {&ShaderEntity::DESCRIPTOR_SET_SHADOWS};

static std::shared_ptr<prosper::UniformResizableBuffer> g_materialSettingsBuffer = nullptr;
static uint32_t g_instanceCount = 0;
static void initialize_material_settings_buffer()
{
	if(g_materialSettingsBuffer)
		return;
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	bufCreateInfo.size = sizeof(ShaderTextured3DBase::MaterialData) *2'048;
	bufCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::TRANSFER_SRC_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT | Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT;
	g_materialSettingsBuffer = prosper::util::create_uniform_resizable_buffer(*c_engine,bufCreateInfo,sizeof(ShaderTextured3DBase::MaterialData),sizeof(ShaderTextured3DBase::MaterialData) *524'288,0.05f);
	g_materialSettingsBuffer->SetPermanentlyMapped(true);
}
ShaderTextured3DBase::ShaderTextured3DBase(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderEntity(context,identifier,vsShader,fsShader,gsShader)
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
	if(g_instanceCount++ == 0u)
		initialize_material_settings_buffer();
}
ShaderTextured3DBase::~ShaderTextured3DBase()
{
	if(--g_instanceCount == 0)
		g_materialSettingsBuffer = nullptr;
}
prosper::Shader::DescriptorSetInfo &ShaderTextured3DBase::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
void ShaderTextured3DBase::InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT);
}
void ShaderTextured3DBase::InitializeGfxPipelineVertexAttributes(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_UV);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_NORMAL);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_TANGENT);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BI_TANGENT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_LIGHTMAP_UV);

	/*if(static_cast<Pipeline>(pipelineIdx) == Pipeline::LightMap)
	{
		AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_LIGHTMAP_UV);
		const auto lightMapEnabled = true;
		//pipelineInfo.add_specialization_constant(Anvil::ShaderStage::FRAGMENT,0u,sizeof(lightMapEnabled),&lightMapEnabled);
		//pipelineInfo.add_specialization_constant(Anvil::ShaderStage::VERTEX,0u,sizeof(lightMapEnabled),&lightMapEnabled);
	}*/
}
void ShaderTextured3DBase::InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,GetMaterialDescriptorSetInfo());
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CSM);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SHADOWS);
}
void ShaderTextured3DBase::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderEntity::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	if(pipelineIdx == umath::to_integral(Pipeline::Reflection))
		prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,Anvil::CullModeFlagBits::FRONT_BIT);

	pipelineInfo.toggle_depth_writes(false);
	pipelineInfo.toggle_depth_test(true,Anvil::CompareOp::LESS_OR_EQUAL);
	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	InitializeGfxPipelineVertexAttributes(pipelineInfo,pipelineIdx);
	InitializeGfxPipelinePushConstantRanges(pipelineInfo,pipelineIdx);
	InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo,true);
}

static auto cvNormalMappingEnabled = GetClientConVar("render_normalmapping_enabled");
bool ShaderTextured3DBase::BindMaterialParameters(CMaterial &mat) {return true;}
void ShaderTextured3DBase::ApplyMaterialFlags(CMaterial &mat,MaterialFlags &outFlags) const {}
bool ShaderTextured3DBase::BindClipPlane(const Vector4 &clipPlane)
{
	umath::set_flag(m_stateFlags,StateFlags::ClipPlaneBound);
	return RecordPushConstants(Vector3(clipPlane.x,clipPlane.y,clipPlane.z) *clipPlane.w,offsetof(PushConstants,clipPlane));
}
void ShaderTextured3DBase::OnPipelineBound()
{
	ShaderEntity::OnPipelineBound();
	umath::set_flag(m_stateFlags,StateFlags::ClipPlaneBound,false);
}
void ShaderTextured3DBase::OnPipelineUnbound()
{
	ShaderEntity::OnPipelineUnbound();
	umath::set_flag(m_stateFlags,StateFlags::ClipPlaneBound,false);
}
bool ShaderTextured3DBase::BeginDraw(const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,Pipeline pipelineIdx,RecordFlags recordFlags)
{
	return ShaderScene::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx),recordFlags) == true &&
		BindClipPlane(clipPlane) == true;
}
void ShaderTextured3DBase::UpdateMaterialBuffer(CMaterial &mat) const
{
	auto *buf = mat.GetSettingsBuffer();
	if(buf == nullptr)
		return;
	MaterialData matData {};

	auto &matFlags = matData.flags;

	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap && diffuseMap->texture && std::static_pointer_cast<Texture>(diffuseMap->texture)->HasFlag(Texture::Flags::SRGB))
		matFlags |= MaterialFlags::DiffuseSRGB;

	auto &data = mat.GetDataBlock();
	auto *parallaxMap = mat.GetParallaxMap();
	if(parallaxMap != nullptr && parallaxMap->texture != nullptr)
	{
		matFlags |= MaterialFlags::Parallax;

		if(data != nullptr)
			data->GetFloat("parallax_height_scale",&matData.parallaxHeightScale);
	}

	if(cvNormalMappingEnabled->GetBool() == true)
	{
		auto *normalMap = mat.GetNormalMap();
		if(normalMap != nullptr && normalMap->texture != nullptr)
			matFlags |= MaterialFlags::Normal;
	}

	auto *specularMap = mat.GetSpecularMap();
	if(specularMap != nullptr && specularMap->texture != nullptr)
		matFlags |= MaterialFlags::SpecularMap;

	if(data != nullptr)
	{
		if(data->GetBool("black_to_alpha") == true)
			matFlags |= MaterialFlags::BlackToAlpha;

		auto &phongColor = data->GetValue("phong_color");
		if(phongColor != nullptr)
		{
			const auto &colorType = typeid(ds::Color);
			if(typeid(*phongColor) == colorType)
			{
				matFlags |= MaterialFlags::Specular;
				auto &col = static_cast<ds::Color*>(phongColor.get())->GetValue();
				matData.phong.x = col.r /255.f;
				matData.phong.y = col.g /255.f;
				matData.phong.z = col.b /255.f;
				matData.phong.w = col.a /255.f;
			}
		}
		if(data->GetFloat("phong_shininess",&matData.phong.w) == true)
			matFlags |= MaterialFlags::Specular;
		if(data->GetFloat("phong_intensity",&matData.phongIntensity) == true)
			matFlags |= MaterialFlags::Specular;
	}

	if(data != nullptr && (matFlags &MaterialFlags::Specular) != MaterialFlags::None && (matFlags &MaterialFlags::SpecularMapDefined) == MaterialFlags::None)
	{
		// Check if diffuse alpha should be used as specular component,
		// but only if phong is enabled and no other specular map has been specified
		if(data->GetBool("phong_diffuse_alpha") == true)
			matFlags |= MaterialFlags::DiffuseSpecular;
		else if((matFlags &MaterialFlags::Normal) != MaterialFlags::None && data->GetBool("phong_normal_alpha") == true)
			matFlags |= MaterialFlags::NormalSpecular;
	}

	auto *glowMap = mat.GetGlowMap();
	if(glowMap != nullptr && glowMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(glowMap->texture);
		if(texture->HasFlag(Texture::Flags::SRGB))
			matFlags |= MaterialFlags::GlowSRGB;
		auto bUseGlow = true;
		if(data->GetBool("glow_alpha_only") == true)
		{
			if(prosper::util::has_alpha(texture->GetVkTexture()->GetImage()->GetFormat()) == false)
				bUseGlow = false;
		}
		if(bUseGlow == true)
		{
			int32_t glowMode = 1;
			if(data != nullptr)
				data->GetInt("glow_blend_diffuse_mode",&glowMode);
			if(glowMode != 0)
			{
				matFlags |= MaterialFlags::Glow;
				if(data != nullptr)
					data->GetFloat("glow_blend_diffuse_scale",&matData.glowScale);
			}
			switch(glowMode)
			{
			case 1:
				matFlags |= MaterialFlags::FMAT_GLOW_MODE_1;
				break;
			case 2:
				matFlags |= MaterialFlags::FMAT_GLOW_MODE_2;
				break;
			case 3:
				matFlags |= MaterialFlags::FMAT_GLOW_MODE_3;
				break;
			case 4:
				matFlags |= MaterialFlags::FMAT_GLOW_MODE_4;
				break;
			}
		}
	}

	if(data != nullptr)
		data->GetFloat("alpha_discard_threshold",&matData.alphaDiscardThreshold);

	matData.metalnessFactor = mat.GetMetalnessMap() ? 1.f : 0.f;
	if(data != nullptr)
		data->GetFloat("metalness_factor",&matData.metalnessFactor);

	if(mat.IsTranslucent() == true)
		matFlags |= MaterialFlags::Translucent;
	ApplyMaterialFlags(mat,matFlags);

	buf->Write(0,matData);
}
bool ShaderTextured3DBase::BindLightMapUvBuffer(CModelSubMesh &mesh,bool &outShouldUseLightmaps)
{
	outShouldUseLightmaps = false;
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShouldUseLightMap) == false)
		return true;
	outShouldUseLightmaps = (mesh.GetReferenceId() != std::numeric_limits<uint32_t>::max()) ? 1u : 0u;
	prosper::Buffer *pLightMapUvBuffer = nullptr;
	auto pLightMapComponent = (m_boundEntity != nullptr) ? m_boundEntity->GetComponent<pragma::CLightMapComponent>() : util::WeakHandle<pragma::CLightMapComponent>{};
	if(pLightMapComponent.valid())
	{
		auto meshIndex = mesh.GetReferenceId();
		auto *pUvBuffer = pLightMapComponent->GetMeshLightMapUvBuffer(meshIndex);
		if(pUvBuffer != nullptr)
			pLightMapUvBuffer = pUvBuffer;
		else
			pLightMapUvBuffer = c_engine->GetDummyBuffer().get();
	}
	else
		pLightMapUvBuffer = c_engine->GetDummyBuffer().get();
	return RecordBindVertexBuffer(pLightMapUvBuffer->GetAnvilBuffer(),umath::to_integral(VertexBinding::LightmapUv));
}
void ShaderTextured3DBase::UpdateRenderFlags(CModelSubMesh &mesh,RenderFlags &inOutFlags) {}
bool ShaderTextured3DBase::Draw(CModelSubMesh &mesh)
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::ClipPlaneBound) == false && BindClipPlane({}) == false)
		return false;
	auto shouldUseLightmaps = false;
	if(BindLightMapUvBuffer(mesh,shouldUseLightmaps) == false)
		return false;
	auto renderFlags = RenderFlags::None;
	umath::set_flag(renderFlags,RenderFlags::LightmapsEnabled,shouldUseLightmaps);
	umath::set_flag(renderFlags,RenderFlags::UseExtendedVertexWeights,mesh.GetExtendedVertexWeights().empty() == false);
	UpdateRenderFlags(mesh,renderFlags);
	return RecordPushConstants(renderFlags,offsetof(ShaderTextured3DBase::PushConstants,flags)) && ShaderEntity::Draw(mesh);
}
size_t ShaderTextured3DBase::GetBaseTypeHashCode() const {return HASH_TYPE;}
uint32_t ShaderTextured3DBase::GetCameraDescriptorSetIndex() const {return DESCRIPTOR_SET_CAMERA.setIndex;}
uint32_t ShaderTextured3DBase::GetInstanceDescriptorSetIndex() const {return DESCRIPTOR_SET_INSTANCE.setIndex;}
uint32_t ShaderTextured3DBase::GetRenderSettingsDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex;}
uint32_t ShaderTextured3DBase::GetLightDescriptorSetIndex() const {return DESCRIPTOR_SET_LIGHTS.setIndex;}
uint32_t ShaderTextured3DBase::GetMaterialDescriptorSetIndex() const {return DESCRIPTOR_SET_MATERIAL.setIndex;}
void ShaderTextured3DBase::GetVertexAnimationPushConstantInfo(uint32_t &offset) const
{
	offset = offsetof(PushConstants,vertexAnimInfo);
}
bool ShaderTextured3DBase::BindMaterial(CMaterial &mat)
{
	auto descSetGroup = mat.GetDescriptorSetGroup(*this);
	if(descSetGroup == nullptr)
		descSetGroup = InitializeMaterialDescriptorSet(mat); // Attempt to initialize on the fly
	if(descSetGroup == nullptr)
		return false;
	return BindMaterialParameters(mat) && RecordBindDescriptorSet(*(*descSetGroup)->get_descriptor_set(0u),GetMaterialDescriptorSetIndex());
}
std::shared_ptr<prosper::DescriptorSetGroup> ShaderTextured3DBase::InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::Shader::DescriptorSetInfo &descSetInfo)
{
	auto &dev = c_engine->GetDevice();
	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap == nullptr || diffuseMap->texture == nullptr)
		return nullptr;
	auto diffuseTexture = std::static_pointer_cast<Texture>(diffuseMap->texture);
	if(diffuseTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = prosper::util::create_descriptor_set_group(dev,descSetInfo);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_texture(descSet,*diffuseTexture->GetVkTexture(),umath::to_integral(MaterialBinding::DiffuseMap));

	auto *normalMap = mat.GetNormalMap();
	if(normalMap != nullptr && normalMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(normalMap->texture);
		if(texture->HasValidVkTexture())
			prosper::util::set_descriptor_set_binding_texture(descSet,*texture->GetVkTexture(),umath::to_integral(MaterialBinding::NormalMap));
	}

	auto *specularMap = mat.GetSpecularMap();
	if(specularMap != nullptr && specularMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(specularMap->texture);
		if(texture->HasValidVkTexture())
			prosper::util::set_descriptor_set_binding_texture(descSet,*texture->GetVkTexture(),umath::to_integral(MaterialBinding::SpecularMap));
	}

	auto *parallaxMap = mat.GetParallaxMap();
	if(parallaxMap != nullptr && parallaxMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(parallaxMap->texture);
		if(texture->HasValidVkTexture())
			prosper::util::set_descriptor_set_binding_texture(descSet,*texture->GetVkTexture(),umath::to_integral(MaterialBinding::ParallaxMap));
	}

	auto *glowMap = mat.GetGlowMap();
	if(glowMap != nullptr && glowMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(glowMap->texture);
		if(texture->HasValidVkTexture())
			prosper::util::set_descriptor_set_binding_texture(descSet,*texture->GetVkTexture(),umath::to_integral(MaterialBinding::GlowMap));
	}
	InitializeMaterialBuffer(descSet,mat);

	return descSetGroup;
}
void ShaderTextured3DBase::InitializeMaterialBuffer(prosper::DescriptorSet &descSet,CMaterial &mat)
{
	auto settingsBuffer = mat.GetSettingsBuffer() ? mat.GetSettingsBuffer()->shared_from_this() : nullptr;
	if(settingsBuffer == nullptr && g_materialSettingsBuffer)
		settingsBuffer = g_materialSettingsBuffer->AllocateBuffer();
	if(settingsBuffer == nullptr)
		return;
	prosper::util::set_descriptor_set_binding_uniform_buffer(descSet,*settingsBuffer,umath::to_integral(MaterialBinding::MaterialSettings));
	mat.SetSettingsBuffer(*settingsBuffer);
	UpdateMaterialBuffer(mat);
}
std::shared_ptr<prosper::DescriptorSetGroup> ShaderTextured3DBase::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	return InitializeMaterialDescriptorSet(mat,DESCRIPTOR_SET_MATERIAL);
}

////////////////////////////

ShaderTextured3D::ShaderTextured3D(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase(context,identifier,"world/vs_textured","world/fs_textured")
{}

static void set_debug_flag(pragma::ShaderScene::DebugFlags setFlags,pragma::ShaderScene::DebugFlags unsetFlags)
{
	auto &debugBuffer = c_game->GetGlobalRenderSettingsBufferData().debugBuffer;

	auto debugFlags = decltype(pragma::ShaderTextured3DBase::DebugData::flags){};
	auto offset = offsetof(pragma::ShaderTextured3DBase::DebugData,flags);
	debugBuffer->Map(offset,sizeof(debugFlags));
	debugBuffer->Read(offset,debugFlags);
	debugFlags |= umath::to_integral(setFlags);
	debugFlags &= ~umath::to_integral(unsetFlags);
	debugBuffer->Write(offset,debugFlags);
}
static void set_debug_flag(pragma::ShaderScene::DebugFlags flag,bool set)
{
	if(set)
		set_debug_flag(flag,pragma::ShaderScene::DebugFlags::None);
	else
		set_debug_flag(pragma::ShaderScene::DebugFlags::None,flag);
}

static CVar cvShowCascades = GetClientConVar("debug_csm_show_cascades");
REGISTER_CONVAR_CALLBACK_CL(debug_csm_show_cascades,[](NetworkState*,ConVar*,bool,bool val) {
	set_debug_flag(pragma::ShaderScene::DebugFlags::LightShowCascades,val);
});

static CVar cvShowLightDepth = GetClientConVar("debug_light_depth");
REGISTER_CONVAR_CALLBACK_CL(debug_light_depth,[](NetworkState*,ConVar*,int,int val) {
	switch(val)
	{
		case 0:
			set_debug_flag(pragma::ShaderScene::DebugFlags::None,pragma::ShaderScene::DebugFlags::LightShowShadowMapDepth | pragma::ShaderScene::DebugFlags::LightShowFragmentDepthShadowSpace);
			break;
		case 1:
			set_debug_flag(pragma::ShaderScene::DebugFlags::LightShowShadowMapDepth,pragma::ShaderScene::DebugFlags::LightShowFragmentDepthShadowSpace);
			break;
		case 2:
			set_debug_flag(pragma::ShaderScene::DebugFlags::LightShowFragmentDepthShadowSpace,pragma::ShaderScene::DebugFlags::LightShowShadowMapDepth);
			break;
	}
});

REGISTER_CONVAR_CALLBACK_CL(debug_forwardplus_heatmap,[](NetworkState*,ConVar*,bool,bool val) {
	set_debug_flag(pragma::ShaderScene::DebugFlags::ForwardPlusHeatmap,val);
});
#pragma optimize("",on)
