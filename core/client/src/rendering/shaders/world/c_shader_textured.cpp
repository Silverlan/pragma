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
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/game/game_limits.h>
#include <datasystem_color.h>
#include <datasystem_vector.h>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_path.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
/*ShaderGameWorldPipeline ShaderGameWorldLightingPass::GetPipelineIndex(prosper::SampleCountFlags sampleCount,bool bReflection)
{
	if(sampleCount == prosper::SampleCountFlags::e1Bit)
		return bReflection ? ShaderGameWorldPipeline::Reflection : ShaderGameWorldPipeline::Regular;
	if(bReflection)
		throw std::logic_error("Multi-sampled reflection pipeline not supported!");
	return ShaderGameWorldPipeline::MultiSample;
}*/

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_RENDER_BUFFER_INDEX) ShaderGameWorldLightingPass::VERTEX_BINDING_RENDER_BUFFER_INDEX = {prosper::VertexInputRate::Instance};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX = {ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX,VERTEX_BINDING_RENDER_BUFFER_INDEX};

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_BONE_WEIGHT) ShaderGameWorldLightingPass::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID,VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT,VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderGameWorldLightingPass::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID,VERTEX_BINDING_BONE_WEIGHT_EXT};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT,VERTEX_BINDING_BONE_WEIGHT_EXT};

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_VERTEX) ShaderGameWorldLightingPass::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex,sizeof(VertexBufferData)};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_POSITION) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_UV) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_UV = {ShaderEntity::VERTEX_ATTRIBUTE_UV,VERTEX_BINDING_VERTEX};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_NORMAL) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_NORMAL = {ShaderEntity::VERTEX_ATTRIBUTE_NORMAL,VERTEX_BINDING_VERTEX};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_TANGENT) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_TANGENT = {ShaderEntity::VERTEX_ATTRIBUTE_TANGENT,VERTEX_BINDING_VERTEX};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BI_TANGENT) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BI_TANGENT = {ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT,VERTEX_BINDING_VERTEX};

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_LIGHTMAP) ShaderGameWorldLightingPass::VERTEX_BINDING_LIGHTMAP = {prosper::VertexInputRate::Vertex};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_LIGHTMAP_UV) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_LIGHTMAP_UV = {ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV,VERTEX_BINDING_LIGHTMAP};

decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE) ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_MATERIAL) ShaderGameWorldLightingPass::DESCRIPTOR_SET_MATERIAL = {
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
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_SCENE) ShaderGameWorldLightingPass::DESCRIPTOR_SET_SCENE = {&ShaderEntity::DESCRIPTOR_SET_SCENE};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDERER) ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDERER = {&ShaderEntity::DESCRIPTOR_SET_RENDERER};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderEntity::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_LIGHTS) ShaderGameWorldLightingPass::DESCRIPTOR_SET_LIGHTS = {&ShaderEntity::DESCRIPTOR_SET_LIGHTS};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_SHADOWS) ShaderGameWorldLightingPass::DESCRIPTOR_SET_SHADOWS = {&ShaderEntity::DESCRIPTOR_SET_SHADOWS};

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
	//bufCreateInfo.size = sizeof(ShaderGameWorldLightingPass::MaterialData) *2'048;
	bufCreateInfo.size = sizeof(ShaderGameWorldLightingPass::MaterialData) *524'288; // ~22 MiB
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit | prosper::BufferUsageFlags::UniformBufferBit;
	bufCreateInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	g_materialSettingsBuffer = c_engine->GetRenderContext().CreateUniformResizableBuffer(bufCreateInfo,sizeof(ShaderGameWorldLightingPass::MaterialData),sizeof(ShaderGameWorldLightingPass::MaterialData) *524'288,0.05f);
	g_materialSettingsBuffer->SetPermanentlyMapped(true,prosper::IBuffer::MapFlags::WriteBit);
}
ShaderGameWorldLightingPass::ShaderGameWorldLightingPass(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderGameWorld(context,identifier,vsShader,fsShader,gsShader)
{
	if(g_instanceCount++ == 0u)
		initialize_material_settings_buffer();

	auto n = umath::to_integral(GameShaderSpecialization::Count);
	for(auto i=decltype(n){0u};i<n;++i)
	{
		auto dynamicFlags = GameShaderSpecializationConstantFlag::None;
		switch(static_cast<GameShaderSpecialization>(i))
		{
		case GameShaderSpecialization::Generic:
			dynamicFlags = GameShaderSpecializationConstantFlag::EmissionEnabledBit | GameShaderSpecializationConstantFlag::EnableRmaMapBit |
				GameShaderSpecializationConstantFlag::EnableNormalMapBit | GameShaderSpecializationConstantFlag::ParallaxEnabledBit | GameShaderSpecializationConstantFlag::EnableDepthBias;
			break;
		case GameShaderSpecialization::Lightmapped:
			dynamicFlags = GameShaderSpecializationConstantFlag::EmissionEnabledBit | GameShaderSpecializationConstantFlag::EnableRmaMapBit |
				GameShaderSpecializationConstantFlag::EnableNormalMapBit | GameShaderSpecializationConstantFlag::ParallaxEnabledBit | GameShaderSpecializationConstantFlag::EnableDepthBias;
			break;
		case GameShaderSpecialization::Animated:
			dynamicFlags = GameShaderSpecializationConstantFlag::EmissionEnabledBit | GameShaderSpecializationConstantFlag::EnableRmaMapBit |
				GameShaderSpecializationConstantFlag::EnableNormalMapBit | GameShaderSpecializationConstantFlag::ParallaxEnabledBit | GameShaderSpecializationConstantFlag::WrinklesEnabledBit |
				GameShaderSpecializationConstantFlag::EnableExtendedVertexWeights | GameShaderSpecializationConstantFlag::EnableDepthBias;
			break;
		}
		auto staticFlags = GetStaticSpecializationConstantFlags(static_cast<GameShaderSpecialization>(i));
		RegisterSpecializations(PassType::Generic,staticFlags,dynamicFlags);
	}

	auto numPipelines = ShaderSpecializationManager::GetPipelineCount();
	SetPipelineCount(numPipelines);
}
ShaderGameWorldLightingPass::~ShaderGameWorldLightingPass()
{
	if(--g_instanceCount == 0)
		g_materialSettingsBuffer = nullptr;
}
GameShaderSpecializationConstantFlag ShaderGameWorldLightingPass::GetStaticSpecializationConstantFlags(GameShaderSpecialization specialization) const
{
	auto staticFlags = 
		GameShaderSpecializationConstantFlag::BloomOutputEnabledBit | GameShaderSpecializationConstantFlag::EnableSsaoBit |
		GameShaderSpecializationConstantFlag::EnableLightSourcesBit | GameShaderSpecializationConstantFlag::EnableLightSourcesDirectionalBit |
		GameShaderSpecializationConstantFlag::EnableLightSourcesPointBit | GameShaderSpecializationConstantFlag::EnableLightSourcesSpotBit;
	switch(specialization)
	{
	case GameShaderSpecialization::Generic:
		break;
	case GameShaderSpecialization::Lightmapped:
		staticFlags |= GameShaderSpecializationConstantFlag::EnableLightMapsBit;
		break;
	case GameShaderSpecialization::Animated:
		staticFlags |= GameShaderSpecializationConstantFlag::EnableAnimationBit | GameShaderSpecializationConstantFlag::EnableMorphTargetAnimationBit;
		break;
	}
	return staticFlags;
}
std::optional<uint32_t> ShaderGameWorldLightingPass::FindPipelineIndex(PassType passType,GameShaderSpecialization specialization,GameShaderSpecializationConstantFlag specializationFlags) const
{
	return ShaderSpecializationManager::FindSpecializationPipelineIndex(passType,GetStaticSpecializationConstantFlags(specialization) | specializationFlags);
}
GameShaderSpecializationConstantFlag ShaderGameWorldLightingPass::GetMaterialPipelineSpecializationRequirements(CMaterial &mat) const
{
	auto flags = GameShaderSpecializationConstantFlag::None;
	if(mat.GetTextureInfo(Material::EMISSION_MAP_IDENTIFIER))
		flags |= GameShaderSpecializationConstantFlag::EmissionEnabledBit;
	if(mat.GetTextureInfo(Material::WRINKLE_STRETCH_MAP_IDENTIFIER) || mat.GetTextureInfo(Material::WRINKLE_COMPRESS_MAP_IDENTIFIER))
		flags |= GameShaderSpecializationConstantFlag::WrinklesEnabledBit;
	if(mat.GetAlphaMode() != AlphaMode::Opaque)
		flags |= GameShaderSpecializationConstantFlag::EnableTranslucencyBit;
	auto *rmaMap = mat.GetRMAMap();
	if(rmaMap)
	{
		flags |= GameShaderSpecializationConstantFlag::EnableRmaMapBit;
		auto &texture = std::static_pointer_cast<Texture>(rmaMap->texture);
		if(texture)
		{
			auto texName = texture->GetName();
			ustring::to_lower(texName);
			auto path = util::Path::CreateFile(texName);
			path.RemoveFileExtension();
			if(path == "pbr/rma_neutral")
			{
				// If the material uses the neutral rma texture, we can completely ignore
				// it for the shader and use the default values instead, which saves
				// a lot of texture lookups.
				flags &= ~GameShaderSpecializationConstantFlag::EnableRmaMapBit;
			}
		}
	}
	if(mat.GetNormalMap())
		flags |= GameShaderSpecializationConstantFlag::EnableNormalMapBit;
	if(mat.GetParallaxMap())
		flags |= GameShaderSpecializationConstantFlag::ParallaxEnabledBit;
	return flags;
}
prosper::DescriptorSetInfo &ShaderGameWorldLightingPass::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
void ShaderGameWorldLightingPass::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}
void ShaderGameWorldLightingPass::InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

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
void ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,GetMaterialDescriptorSetInfo());
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDERER);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SHADOWS);
}
void ShaderGameWorldLightingPass::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderEntity::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	//if(pipelineIdx == umath::to_integral(ShaderGameWorldPipeline::Reflection))
	//	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::FrontBit);

	// TODO: Technically we shouldn't have to write depth values, since
	// they've already been written in the depth prepass, but that causes
	// visual glitches for translucent objects. Find the cause!
	pipelineInfo.ToggleDepthWrites(true);
	pipelineInfo.ToggleDepthTest(true,prosper::CompareOp::LessOrEqual);

	//pipelineInfo.ToggleDepthBias(true,0.f,0.f,0.f);
	pipelineInfo.ToggleDynamicState(true,prosper::DynamicState::DepthBias); // Required for decals

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	InitializeGfxPipelineVertexAttributes(pipelineInfo,pipelineIdx);
	InitializeGfxPipelinePushConstantRanges(pipelineInfo,pipelineIdx);
	InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo,true);
}

static auto cvNormalMappingEnabled = GetClientConVar("render_normalmapping_enabled");
bool ShaderGameWorldLightingPass::BindMaterialParameters(CMaterial &mat) {return true;}
void ShaderGameWorldLightingPass::ApplyMaterialFlags(CMaterial &mat,MaterialFlags &outFlags) const {}
bool ShaderGameWorldLightingPass::BindReflectionProbeIntensity(float intensity)
{
	return RecordPushConstants(intensity,offsetof(PushConstants,reflectionProbeIntensity));
}
bool ShaderGameWorldLightingPass::BindClipPlane(const Vector4 &clipPlane)
{
	// TODO
	//umath::set_flag(m_sceneFlags,SceneFlags::Cli);
	return RecordPushConstants(clipPlane,offsetof(PushConstants,clipPlane));
}
void ShaderGameWorldLightingPass::Set3DSky(bool is3dSky) {umath::set_flag(m_sceneFlags,SceneFlags::RenderAs3DSky,is3dSky);}
void ShaderGameWorldLightingPass::SetShadowsEnabled(bool enabled) {umath::set_flag(m_sceneFlags,SceneFlags::DisableShadows,!enabled);}
void ShaderGameWorldLightingPass::OnPipelineBound()
{
	// TODO
	ShaderEntity::OnPipelineBound();
	//umath::set_flag(m_sceneFlags,SceneFlags::ClipPlaneBound,false);
}
void ShaderGameWorldLightingPass::OnPipelineUnbound()
{
	// TODO
	ShaderEntity::OnPipelineUnbound();
	//umath::set_flag(m_sceneFlags,SceneFlags::ClipPlaneBound,false);
}
void ShaderGameWorldLightingPass::OnBindEntity(CBaseEntity &ent,CRenderComponent &renderC)
{
	ShaderEntity::OnBindEntity(ent,renderC);
	SetShadowsEnabled(renderC.IsReceivingShadows());
}
bool ShaderGameWorldLightingPass::BindDrawOrigin(const Vector4 &drawOrigin) {return RecordPushConstants(drawOrigin,offsetof(PushConstants,drawOrigin));}
bool ShaderGameWorldLightingPass::SetDepthBias(const Vector2 &depthBias) {return RecordPushConstants(depthBias,offsetof(PushConstants,depthBias));}
bool ShaderGameWorldLightingPass::BeginDraw(
	const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin,RecordFlags recordFlags
)
{
	Set3DSky(false);
	return ShaderScene::BeginDraw(cmdBuffer,0u,recordFlags) == true &&
		BindClipPlane(clipPlane) == true &&
		RecordPushConstants(drawOrigin,offsetof(PushConstants,drawOrigin)) &&
		RecordPushConstants(Vector2{},offsetof(PushConstants,depthBias)) &&
		RecordPushConstants(pragma::SceneDebugMode::None,offsetof(PushConstants,debugMode)) &&
		cmdBuffer->RecordSetDepthBias() == true;
}
bool ShaderGameWorldLightingPass::SetDebugMode(pragma::SceneDebugMode debugMode)
{
	return RecordPushConstants(debugMode,offsetof(PushConstants,debugMode));
}
std::optional<ShaderGameWorldLightingPass::MaterialData> ShaderGameWorldLightingPass::UpdateMaterialBuffer(CMaterial &mat) const
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

	matData.color = {1.f,1.f,1.f,1.f};
	data->GetVector3("color_factor",reinterpret_cast<Vector3*>(&matData.color));
	data->GetFloat("alpha_factor",&matData.color.a);

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
bool ShaderGameWorldLightingPass::BindLightMapUvBuffer(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,bool &outShouldUseLightmaps)
{
	outShouldUseLightmaps = false;
	if(umath::is_flag_set(m_sceneFlags,SceneFlags::LightmapsEnabled) == false)
		return true;
	auto *pLightMapUvBuffer = c_engine->GetRenderContext().GetDummyBuffer().get();
	if(m_boundEntity)
	{
		auto *renderC = m_boundEntity->GetRenderComponent();
		if(renderC)
		{
			auto *lightMapReceiverC = renderC->GetLightMapReceiverComponent();
			auto bufIdx = lightMapReceiverC ? (meshIdx.has_value() ? lightMapReceiverC->GetBufferIndex(*meshIdx) : lightMapReceiverC->FindBufferIndex(mesh)) : std::optional<uint32_t>{};
			if(bufIdx.has_value())
			{
				outShouldUseLightmaps = true;

				// Commented because vertex buffers are now bound through render buffer
#if 0
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
#endif
			}
		}
	}
	// Commented because vertex buffers are now bound through render buffer
	// TODO: Restructure this function and clean this up!
	return true;//RecordBindVertexBuffer(*pLightMapUvBuffer,umath::to_integral(VertexBinding::LightmapUv));
}
void ShaderGameWorldLightingPass::UpdateRenderFlags(CModelSubMesh &mesh,SceneFlags &inOutFlags) {}
bool ShaderGameWorldLightingPass::BindRenderFlags(SceneFlags flags)
{
	return RecordPushConstants(flags,offsetof(ShaderGameWorldLightingPass::PushConstants,flags));
}
bool ShaderGameWorldLightingPass::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount)
{
	/*if(umath::is_flag_set(m_stateFlags,SceneFlags::ClipPlaneBound) == false && BindClipPlane({}) == false)
		return false;
	auto shouldUseLightmaps = false;
	if(BindLightMapUvBuffer(mesh,meshIdx,shouldUseLightmaps) == false)
		return false;
	auto renderFlags = RenderFlags::None;
	umath::set_flag(renderFlags,RenderFlags::LightmapsEnabled,shouldUseLightmaps);
	umath::set_flag(renderFlags,RenderFlags::UseExtendedVertexWeights,mesh.GetExtendedVertexWeights().empty() == false);
	if(umath::is_flag_set(m_stateFlags,StateFlags::RenderAs3DSky))
		umath::set_flag(renderFlags,RenderFlags::Is3DSky);
	if(umath::is_flag_set(m_stateFlags,StateFlags::DisableShadows))
		umath::set_flag(renderFlags,RenderFlags::DisableShadows);
	UpdateRenderFlags(mesh,renderFlags);
	return BindRenderFlags(renderFlags) && ShaderEntity::Draw(mesh,meshIdx,renderBufferIndexBuffer,instanceCount);*/
	return true; // TODO
}
bool ShaderGameWorldLightingPass::GetRenderBufferTargets(
	CModelSubMesh &mesh,uint32_t pipelineIdx,std::vector<prosper::IBuffer*> &outBuffers,std::vector<prosper::DeviceSize> &outOffsets,
	std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo
) const
{
	if(ShaderEntity::GetRenderBufferTargets(mesh,pipelineIdx,outBuffers,outOffsets,outIndexBufferInfo) == false)
		return false;
	auto &sceneMesh = mesh.GetSceneMesh();
	auto *lightmapUvBuf = sceneMesh->GetLightmapUvBuffer().get();
	outBuffers.push_back(lightmapUvBuf);
	outOffsets.push_back(0ull);
	return true;
}
uint32_t ShaderGameWorldLightingPass::GetCameraDescriptorSetIndex() const {return DESCRIPTOR_SET_SCENE.setIndex;}
uint32_t ShaderGameWorldLightingPass::GetRendererDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDERER.setIndex;}
uint32_t ShaderGameWorldLightingPass::GetInstanceDescriptorSetIndex() const {return DESCRIPTOR_SET_INSTANCE.setIndex;}
uint32_t ShaderGameWorldLightingPass::GetRenderSettingsDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex;}
uint32_t ShaderGameWorldLightingPass::GetLightDescriptorSetIndex() const {return DESCRIPTOR_SET_LIGHTS.setIndex;}
uint32_t ShaderGameWorldLightingPass::GetMaterialDescriptorSetIndex() const {return GetMaterialDescriptorSetInfo().setIndex;}
void ShaderGameWorldLightingPass::GetVertexAnimationPushConstantInfo(uint32_t &offset) const
{
	offset = offsetof(PushConstants,vertexAnimInfo);
}
bool ShaderGameWorldLightingPass::BindMaterial(CMaterial &mat)
{
	auto descSetGroup = mat.GetDescriptorSetGroup(*this);
	if(descSetGroup == nullptr)
		descSetGroup = InitializeMaterialDescriptorSet(mat); // Attempt to initialize on the fly
	if(descSetGroup == nullptr)
		return false;
	return BindMaterialParameters(mat) && RecordBindDescriptorSet(*descSetGroup->GetDescriptorSet(),GetMaterialDescriptorSetIndex());
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo)
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
std::optional<ShaderGameWorldLightingPass::MaterialData> ShaderGameWorldLightingPass::InitializeMaterialBuffer(prosper::IDescriptorSet &descSet,CMaterial &mat)
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
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	return InitializeMaterialDescriptorSet(mat,DESCRIPTOR_SET_MATERIAL);
}

////////

bool ShaderSpecializationManager::IsSpecializationConstantSet(uint32_t pipelineIdx,SpecializationFlags flag) const
{
	if(pipelineIdx >= m_pipelineSpecializations.size())
		return false;
	auto flags = m_pipelineSpecializations[pipelineIdx];
	return (flags &flag) != 0;
}
void ShaderSpecializationManager::RegisterSpecializations(PassType passType,SpecializationFlags staticFlags,SpecializationFlags dynamicFlags)
{
	if(passType >= m_specializationToPipelineIdx.size())
		m_specializationToPipelineIdx.resize(passType +1);
	auto &specializationMap = m_specializationToPipelineIdx[passType];
	auto dynamicFlagValues = umath::get_power_of_2_values(dynamicFlags);
	auto &permutations = m_pipelineSpecializations;
	std::function<void(uint32_t,SpecializationFlags)> registerSpecialization = nullptr;
	permutations.reserve(umath::pow(static_cast<size_t>(2),dynamicFlagValues.size()));
	registerSpecialization = [&specializationMap,&registerSpecialization,&dynamicFlagValues,&permutations](uint32_t idx,SpecializationFlags perm) {
		if(idx >= dynamicFlagValues.size())
		{
			permutations.push_back(perm);
			specializationMap[perm] = permutations.size() -1;
			return;
		}
		auto curFlag = dynamicFlagValues[idx];
		registerSpecialization(idx +1,perm);
		registerSpecialization(idx +1,perm |= curFlag);
	};
	registerSpecialization(0,staticFlags);
}
std::optional<uint32_t> ShaderSpecializationManager::FindSpecializationPipelineIndex(PassType passType,uint64_t specializationFlags) const
{
	if(passType >= m_specializationToPipelineIdx.size())
		return {};
	auto specToPipelineIdx = m_specializationToPipelineIdx[passType];
	auto itp = specToPipelineIdx.find(specializationFlags);
	return (itp != specToPipelineIdx.end()) ? itp->second : std::optional<uint32_t>{};
}

////////

static void set_debug_flag(pragma::ShaderScene::DebugFlags setFlags,pragma::ShaderScene::DebugFlags unsetFlags)
{
	auto &debugBuffer = c_game->GetGlobalRenderSettingsBufferData().debugBuffer;

	auto debugFlags = decltype(pragma::ShaderGameWorldLightingPass::DebugData::flags){};
	auto offset = offsetof(pragma::ShaderGameWorldLightingPass::DebugData,flags);
	debugBuffer->Map(offset,sizeof(debugFlags),prosper::IBuffer::MapFlags::ReadBit | prosper::IBuffer::MapFlags::WriteBit);
	debugBuffer->Read(offset,debugFlags);
	debugFlags |= umath::to_integral(setFlags);
	debugFlags &= ~umath::to_integral(unsetFlags);
	debugBuffer->Write(offset,debugFlags);
	debugBuffer->Unmap();
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
