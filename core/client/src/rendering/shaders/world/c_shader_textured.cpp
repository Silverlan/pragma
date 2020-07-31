/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/game/game_limits.h>
#include <datasystem_color.h>
#include <datasystem_vector.h>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;



ShaderTextured3DBase::Pipeline ShaderTextured3DBase::GetPipelineIndex(prosper::SampleCountFlags sampleCount,bool bReflection)
{
	if(sampleCount == prosper::SampleCountFlags::e1Bit)
		return bReflection ? Pipeline::Reflection : Pipeline::Regular;
	if(bReflection)
		throw std::logic_error("Multi-sampled reflection pipeline not supported!");
	return Pipeline::MultiSample;
}

decltype(ShaderTextured3DBase::HASH_TYPE) ShaderTextured3DBase::HASH_TYPE = typeid(ShaderTextured3DBase).hash_code();
decltype(ShaderTextured3DBase::VERTEX_BINDING_BONE_WEIGHT) ShaderTextured3DBase::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID,VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT,VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderTextured3DBase::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderTextured3DBase::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID,VERTEX_BINDING_BONE_WEIGHT_EXT};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT,VERTEX_BINDING_BONE_WEIGHT_EXT};

decltype(ShaderTextured3DBase::VERTEX_BINDING_VERTEX) ShaderTextured3DBase::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex,sizeof(VertexBufferData)};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_POSITION) ShaderTextured3DBase::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_UV) ShaderTextured3DBase::VERTEX_ATTRIBUTE_UV = {ShaderEntity::VERTEX_ATTRIBUTE_UV,VERTEX_BINDING_VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_NORMAL) ShaderTextured3DBase::VERTEX_ATTRIBUTE_NORMAL = {ShaderEntity::VERTEX_ATTRIBUTE_NORMAL,VERTEX_BINDING_VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_TANGENT) ShaderTextured3DBase::VERTEX_ATTRIBUTE_TANGENT = {ShaderEntity::VERTEX_ATTRIBUTE_TANGENT,VERTEX_BINDING_VERTEX};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_BI_TANGENT) ShaderTextured3DBase::VERTEX_ATTRIBUTE_BI_TANGENT = {ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT,VERTEX_BINDING_VERTEX};

decltype(ShaderTextured3DBase::VERTEX_BINDING_LIGHTMAP) ShaderTextured3DBase::VERTEX_BINDING_LIGHTMAP = {prosper::VertexInputRate::Vertex};
decltype(ShaderTextured3DBase::VERTEX_ATTRIBUTE_LIGHTMAP_UV) ShaderTextured3DBase::VERTEX_ATTRIBUTE_LIGHTMAP_UV = {ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV,VERTEX_BINDING_LIGHTMAP};

decltype(ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE) ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_MATERIAL) ShaderTextured3DBase::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::DescriptorSetInfo::Binding { // Material settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::GeometryBit
		},
		prosper::DescriptorSetInfo::Binding { // Diffuse Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Normal Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Specular Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Parallax Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Glow Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA) ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA = {&ShaderEntity::DESCRIPTOR_SET_CAMERA};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_RENDERER) ShaderTextured3DBase::DESCRIPTOR_SET_RENDERER = {&ShaderEntity::DESCRIPTOR_SET_RENDERER};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderTextured3DBase::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderEntity::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_LIGHTS) ShaderTextured3DBase::DESCRIPTOR_SET_LIGHTS = {&ShaderEntity::DESCRIPTOR_SET_LIGHTS};
decltype(ShaderTextured3DBase::DESCRIPTOR_SET_SHADOWS) ShaderTextured3DBase::DESCRIPTOR_SET_SHADOWS = {&ShaderEntity::DESCRIPTOR_SET_SHADOWS};

static std::shared_ptr<prosper::IUniformResizableBuffer> g_materialSettingsBuffer = nullptr;
static uint32_t g_instanceCount = 0;
static void initialize_material_settings_buffer()
{
	if(g_materialSettingsBuffer)
		return;
	// Note: Using a uniform resizable buffer for this doesn't work, because the buffers are used by
	// descriptor sets, which would have to be updated whenever the buffer is re-allocated (which currently
	// does not happen automatically). TODO: Implement this? On the other hand, material data
	// isn't that big to begin with, so maybe just make sure the buffer is large enough for all use cases?
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	//bufCreateInfo.size = sizeof(ShaderTextured3DBase::MaterialData) *2'048;
	bufCreateInfo.size = sizeof(ShaderTextured3DBase::MaterialData) *524'288; // ~22 MiB
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit | prosper::BufferUsageFlags::UniformBufferBit;
	bufCreateInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	g_materialSettingsBuffer = c_engine->GetRenderContext().CreateUniformResizableBuffer(bufCreateInfo,sizeof(ShaderTextured3DBase::MaterialData),sizeof(ShaderTextured3DBase::MaterialData) *524'288,0.05f);
	g_materialSettingsBuffer->SetPermanentlyMapped(true);
}
ShaderTextured3DBase::ShaderTextured3DBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
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
prosper::DescriptorSetInfo &ShaderTextured3DBase::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
void ShaderTextured3DBase::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}
void ShaderTextured3DBase::InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
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
		//pipelineInfo.add_specialization_constant(prosper::ShaderStage::FRAGMENT,0u,sizeof(lightMapEnabled),&lightMapEnabled);
		//pipelineInfo.add_specialization_constant(prosper::ShaderStage::VERTEX,0u,sizeof(lightMapEnabled),&lightMapEnabled);
	}*/
}
void ShaderTextured3DBase::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDERER);
	AddDescriptorSetGroup(pipelineInfo,GetMaterialDescriptorSetInfo());
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SHADOWS);
}
void ShaderTextured3DBase::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderEntity::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	if(pipelineIdx == umath::to_integral(Pipeline::Reflection))
		prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::FrontBit);

	pipelineInfo.ToggleDepthWrites(false);
	pipelineInfo.ToggleDepthTest(true,prosper::CompareOp::LessOrEqual);

	pipelineInfo.ToggleDepthBias(true,0.f,0.f,0.f);
	pipelineInfo.ToggleDynamicState(true,prosper::DynamicState::DepthBias); // Required for decals

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	InitializeGfxPipelineVertexAttributes(pipelineInfo,pipelineIdx);
	InitializeGfxPipelinePushConstantRanges(pipelineInfo,pipelineIdx);
	InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo,true);
}

static auto cvNormalMappingEnabled = GetClientConVar("render_normalmapping_enabled");
bool ShaderTextured3DBase::BindMaterialParameters(CMaterial &mat) {return true;}
void ShaderTextured3DBase::ApplyMaterialFlags(CMaterial &mat,MaterialFlags &outFlags) const {}
bool ShaderTextured3DBase::BindReflectionProbeIntensity(float intensity)
{
	return RecordPushConstants(intensity,offsetof(PushConstants,clipPlane) +sizeof(Vector3));
}
bool ShaderTextured3DBase::BindClipPlane(const Vector4 &clipPlane)
{
	umath::set_flag(m_stateFlags,StateFlags::ClipPlaneBound);
	return RecordPushConstants(Vector3(clipPlane.x,clipPlane.y,clipPlane.z) *clipPlane.w,offsetof(PushConstants,clipPlane));
}
void ShaderTextured3DBase::Set3DSky(bool is3dSky) {umath::set_flag(m_stateFlags,StateFlags::RenderAs3DSky,is3dSky);}
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
bool ShaderTextured3DBase::BeginDraw(
	const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin,Pipeline pipelineIdx,RecordFlags recordFlags
)
{
	Set3DSky(false);
	return ShaderScene::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx),recordFlags) == true &&
		BindClipPlane(clipPlane) == true &&
		RecordPushConstants(drawOrigin,offsetof(PushConstants,drawOrigin)) &&
		RecordPushConstants(Scene::DebugMode::None,offsetof(PushConstants,debugMode)) &&
		cmdBuffer->RecordSetDepthBias() == true;
}
bool ShaderTextured3DBase::SetDebugMode(Scene::DebugMode debugMode)
{
	return RecordPushConstants(debugMode,offsetof(PushConstants,debugMode));
}
std::optional<ShaderTextured3DBase::MaterialData> ShaderTextured3DBase::UpdateMaterialBuffer(CMaterial &mat) const
{
	auto *buf = mat.GetSettingsBuffer();
	if(buf == nullptr)
		return {};
	MaterialData matData {};

	auto &matFlags = matData.flags;

	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap && diffuseMap->texture && std::static_pointer_cast<Texture>(diffuseMap->texture)->HasFlag(Texture::Flags::SRGB))
		matFlags |= MaterialFlags::DiffuseSRGB;

	auto &data = mat.GetDataBlock();
	if(data == nullptr)
		return {};
	auto *parallaxMap = mat.GetParallaxMap();
	if(parallaxMap != nullptr && parallaxMap->texture != nullptr)
	{
		matFlags |= MaterialFlags::Parallax;

		data->GetFloat("parallax_height_scale",&matData.parallaxHeightScale);
	}

	if(cvNormalMappingEnabled->GetBool() == true)
	{
		auto *normalMap = mat.GetNormalMap();
		if(normalMap != nullptr && normalMap->texture != nullptr)
			matFlags |= MaterialFlags::Normal;
	}

	if(data->GetBool("black_to_alpha") == true)
		matFlags |= MaterialFlags::BlackToAlpha;

	auto &colorFactor = data->GetValue("color_factor");
	if(colorFactor != nullptr && typeid(*colorFactor) == typeid(ds::Vector4))
		matData.color = static_cast<ds::Vector4*>(colorFactor.get())->GetValue();

	auto *glowMap = mat.GetGlowMap();
	if(glowMap != nullptr && glowMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(glowMap->texture);

		auto &emissionFactor = data->GetValue("emission_factor");
		if(emissionFactor != nullptr && typeid(*emissionFactor) == typeid(ds::Vector))
		{
			auto &f = static_cast<ds::Vector*>(emissionFactor.get())->GetValue();
			matData.emissionFactor = {f.r,f.g,f.b,1.f};
		}

		if(texture->HasFlag(Texture::Flags::SRGB))
			matFlags |= MaterialFlags::GlowSRGB;
		auto bUseGlow = true;
		if(data->GetBool("glow_alpha_only") == true)
		{
			if(prosper::util::has_alpha(texture->GetVkTexture()->GetImage().GetFormat()) == false)
				bUseGlow = false;
		}
		if(bUseGlow == true)
		{
			int32_t glowMode = 1;
			data->GetInt("glow_blend_diffuse_mode",&glowMode);
			if(glowMode != 0)
			{
				matFlags |= MaterialFlags::Glow;
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

	data->GetFloat("alpha_discard_threshold",&matData.alphaDiscardThreshold);

	auto hasRmaMap = (data->GetValue(Material::RMA_MAP_IDENTIFIER) != nullptr);
	auto defaultMetalness = hasRmaMap ? 1.f : 0.f;
	auto defaultRoughness = hasRmaMap ? 1.f : 0.5f;
	auto defaultAo = hasRmaMap ? 1.f : 0.f;

	matData.metalnessFactor = defaultMetalness;
	data->GetFloat("metalness_factor",&matData.metalnessFactor);

	matData.roughnessFactor = defaultRoughness;
	auto hasRoughnessFactor = data->GetFloat("roughness_factor",&matData.roughnessFactor);

	matData.aoFactor = defaultAo;
	data->GetFloat("ao_factor",&matData.aoFactor);

	float specularFactor;
	if(data->GetFloat("specular_factor",&specularFactor))
	{
		if(hasRoughnessFactor == false)
			matData.roughnessFactor = 1.f;
		matData.roughnessFactor *= (1.f -specularFactor);
	}

	auto alphaMode = static_cast<int32_t>(AlphaMode::Opaque);
	data->GetInt("alpha_mode",&alphaMode);
	matData.alphaMode = static_cast<AlphaMode>(alphaMode);

	auto alphaCutoff = 0.5f;
	data->GetFloat("alpha_cutoff",&alphaCutoff);
	matData.alphaCutoff = alphaCutoff;
	// Obsolete
	//if(mat.IsTranslucent() == true)
	//	matFlags |= MaterialFlags::Translucent;

	ApplyMaterialFlags(mat,matFlags);

	buf->Write(0,matData);
	return matData;
}
bool ShaderTextured3DBase::BindLightMapUvBuffer(CModelSubMesh &mesh,bool &outShouldUseLightmaps)
{
	outShouldUseLightmaps = false;
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShouldUseLightMap) == false)
		return true;
	auto *pLightMapUvBuffer = c_engine->GetRenderContext().GetDummyBuffer().get();
	if(m_boundEntity)
	{
		auto lightMapReceiverC = m_boundEntity->GetComponent<CLightMapReceiverComponent>();
		auto bufIdx = lightMapReceiverC.valid() ? lightMapReceiverC->FindBufferIndex(mesh) : std::optional<uint32_t>{};
		if(bufIdx.has_value())
		{
			outShouldUseLightmaps = true;

			auto *world = c_game->GetWorld();
			auto pLightMapComponent = world ? world->GetEntity().GetComponent<pragma::CLightMapComponent>() : util::WeakHandle<pragma::CLightMapComponent>{};
			//auto pLightMapComponent = (m_boundEntity != nullptr) ? m_boundEntity->GetComponent<pragma::CLightMapComponent>() : util::WeakHandle<pragma::CLightMapComponent>{};
			if(pLightMapComponent.valid())
			{
				auto *pUvBuffer = pLightMapComponent->GetMeshLightMapUvBuffer(*bufIdx);
				if(pUvBuffer != nullptr)
					pLightMapUvBuffer = pUvBuffer;
				else
					pLightMapUvBuffer = c_engine->GetRenderContext().GetDummyBuffer().get();
			}
		}
	}
	return RecordBindVertexBuffer(*pLightMapUvBuffer,umath::to_integral(VertexBinding::LightmapUv));
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
	if(umath::is_flag_set(m_stateFlags,StateFlags::RenderAs3DSky))
		umath::set_flag(renderFlags,RenderFlags::Is3DSky);
	UpdateRenderFlags(mesh,renderFlags);
	return RecordPushConstants(renderFlags,offsetof(ShaderTextured3DBase::PushConstants,flags)) && ShaderEntity::Draw(mesh);
}
size_t ShaderTextured3DBase::GetBaseTypeHashCode() const {return HASH_TYPE;}
uint32_t ShaderTextured3DBase::GetCameraDescriptorSetIndex() const {return DESCRIPTOR_SET_CAMERA.setIndex;}
uint32_t ShaderTextured3DBase::GetRendererDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDERER.setIndex;}
uint32_t ShaderTextured3DBase::GetInstanceDescriptorSetIndex() const {return DESCRIPTOR_SET_INSTANCE.setIndex;}
uint32_t ShaderTextured3DBase::GetRenderSettingsDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex;}
uint32_t ShaderTextured3DBase::GetLightDescriptorSetIndex() const {return DESCRIPTOR_SET_LIGHTS.setIndex;}
uint32_t ShaderTextured3DBase::GetMaterialDescriptorSetIndex() const {return GetMaterialDescriptorSetInfo().setIndex;}
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
	return BindMaterialParameters(mat) && RecordBindDescriptorSet(*descSetGroup->GetDescriptorSet(),GetMaterialDescriptorSetIndex());
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderTextured3DBase::InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo)
{
	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap == nullptr || diffuseMap->texture == nullptr)
		return nullptr;
	auto diffuseTexture = std::static_pointer_cast<Texture>(diffuseMap->texture);
	if(diffuseTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(descSetInfo);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*diffuseTexture->GetVkTexture(),umath::to_integral(MaterialBinding::DiffuseMap));

	auto *normalMap = mat.GetNormalMap();
	if(normalMap != nullptr && normalMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(normalMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(),umath::to_integral(MaterialBinding::NormalMap));
	}

	auto *parallaxMap = mat.GetParallaxMap();
	if(parallaxMap != nullptr && parallaxMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(parallaxMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(),umath::to_integral(MaterialBinding::ParallaxMap));
	}

	auto *glowMap = mat.GetGlowMap();
	if(glowMap != nullptr && glowMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(glowMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(),umath::to_integral(MaterialBinding::GlowMap));
	}
	InitializeMaterialBuffer(descSet,mat);

	return descSetGroup;
}
std::optional<ShaderTextured3DBase::MaterialData> ShaderTextured3DBase::InitializeMaterialBuffer(prosper::IDescriptorSet &descSet,CMaterial &mat)
{
	auto settingsBuffer = mat.GetSettingsBuffer() ? mat.GetSettingsBuffer()->shared_from_this() : nullptr;
	if(settingsBuffer == nullptr && g_materialSettingsBuffer)
		settingsBuffer = g_materialSettingsBuffer->AllocateBuffer();
	if(settingsBuffer == nullptr)
		return {};
	descSet.SetBindingUniformBuffer(*settingsBuffer,umath::to_integral(MaterialBinding::MaterialSettings));
	mat.SetSettingsBuffer(*settingsBuffer);
	return UpdateMaterialBuffer(mat);
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderTextured3DBase::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	return InitializeMaterialDescriptorSet(mat,DESCRIPTOR_SET_MATERIAL);
}

////////

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

