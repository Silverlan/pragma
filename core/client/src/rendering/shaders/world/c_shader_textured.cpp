/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/game/game_limits.h>
#include <pragma/logging.hpp>
#include <datasystem_color.h>
#include <datasystem_vector.h>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/magic_enum.hpp>
#include <sharedutils/util_path.hpp>
#include <util_image.hpp>
#include <cmaterial.h>
#include <cmaterial_manager2.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

/*ShaderGameWorldPipeline ShaderGameWorldLightingPass::GetPipelineIndex(prosper::SampleCountFlags sampleCount,bool bReflection)
{
	if(sampleCount == prosper::SampleCountFlags::e1Bit)
		return bReflection ? ShaderGameWorldPipeline::Reflection : ShaderGameWorldPipeline::Regular;
	if(bReflection)
		throw std::logic_error("Multi-sampled reflection pipeline not supported!");
	return ShaderGameWorldPipeline::MultiSample;
}*/

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_RENDER_BUFFER_INDEX) ShaderGameWorldLightingPass::VERTEX_BINDING_RENDER_BUFFER_INDEX = {prosper::VertexInputRate::Instance};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX = {ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX, VERTEX_BINDING_RENDER_BUFFER_INDEX};

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_BONE_WEIGHT) ShaderGameWorldLightingPass::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID, VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT, VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderGameWorldLightingPass::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID, VERTEX_BINDING_BONE_WEIGHT_EXT};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT, VERTEX_BINDING_BONE_WEIGHT_EXT};

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_VERTEX) ShaderGameWorldLightingPass::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex, sizeof(VertexBufferData)};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_POSITION) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION, VERTEX_BINDING_VERTEX};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_UV) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_UV = {ShaderEntity::VERTEX_ATTRIBUTE_UV, VERTEX_BINDING_VERTEX};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_NORMAL) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_NORMAL = {ShaderEntity::VERTEX_ATTRIBUTE_NORMAL, VERTEX_BINDING_VERTEX};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_TANGENT) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_TANGENT = {ShaderEntity::VERTEX_ATTRIBUTE_TANGENT, VERTEX_BINDING_VERTEX};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BI_TANGENT) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_BI_TANGENT = {ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT, VERTEX_BINDING_VERTEX};

decltype(ShaderGameWorldLightingPass::VERTEX_BINDING_LIGHTMAP) ShaderGameWorldLightingPass::VERTEX_BINDING_LIGHTMAP = {prosper::VertexInputRate::Vertex};
decltype(ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_LIGHTMAP_UV) ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_LIGHTMAP_UV = {ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV, VERTEX_BINDING_LIGHTMAP};

decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE) ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_MATERIAL) ShaderGameWorldLightingPass::DESCRIPTOR_SET_MATERIAL = {{prosper::DescriptorSetInfo::Binding {// Material settings
                                                                                                                          prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::GeometryBit},
  prosper::DescriptorSetInfo::Binding {// Diffuse Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Normal Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Specular Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Parallax Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Glow Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}}};
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
	bufCreateInfo.size = sizeof(ShaderGameWorldLightingPass::MaterialData) * 524'288; // ~22 MiB
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit | prosper::BufferUsageFlags::UniformBufferBit;
	bufCreateInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	g_materialSettingsBuffer = c_engine->GetRenderContext().CreateUniformResizableBuffer(bufCreateInfo, sizeof(ShaderGameWorldLightingPass::MaterialData), sizeof(ShaderGameWorldLightingPass::MaterialData) * 524'288, 0.05f);
	g_materialSettingsBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);
}
ShaderGameWorldLightingPass::ShaderGameWorldLightingPass(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderGameWorld(context, identifier, vsShader, fsShader, gsShader)
{
	if(g_instanceCount++ == 0u)
		initialize_material_settings_buffer();

	auto n = umath::to_integral(GameShaderSpecialization::Count);
	auto nPassTypes = umath::to_integral(rendering::PassType::Count);
	for(auto j = decltype(nPassTypes) {0u}; j < nPassTypes; ++j) {
		// Note: Every pass type has to have the exact same number of pipelines in the exact same order!
		auto startIdx = ShaderSpecializationManager::GetPipelineCount();
		for(auto i = decltype(n) {0u}; i < n; ++i) {
			auto dynamicFlags = GameShaderSpecializationConstantFlag::EnableTranslucencyBit;
			switch(static_cast<GameShaderSpecialization>(i)) {
			case GameShaderSpecialization::Generic:
				break;
			case GameShaderSpecialization::Lightmapped:
				break;
			case GameShaderSpecialization::Animated:
				// dynamicFlags |= GameShaderSpecializationConstantFlag::WrinklesEnabledBit | GameShaderSpecializationConstantFlag::EnableExtendedVertexWeights;
				break;
			}
			auto staticFlags = GetStaticSpecializationConstantFlags(static_cast<GameShaderSpecialization>(i));
			RegisterSpecializations(static_cast<rendering::PassType>(j), staticFlags, dynamicFlags);
		}
		auto endIdx = ShaderSpecializationManager::GetPipelineCount();
		SetPipelineIndexRange(j, startIdx, endIdx);
	}

	auto numPipelines = ShaderSpecializationManager::GetPipelineCount();
	SetPipelineCount(numPipelines);
}
ShaderGameWorldLightingPass::~ShaderGameWorldLightingPass()
{
	if(--g_instanceCount == 0)
		g_materialSettingsBuffer = nullptr;
}
uint32_t ShaderGameWorldLightingPass::GetPassPipelineIndexStartOffset(rendering::PassType passType) const { return GetPipelineIndexStartOffset(umath::to_integral(passType)); }
void ShaderGameWorldLightingPass::OnPipelinesInitialized()
{
	ShaderGameWorld::OnPipelinesInitialized();
	m_defaultMatDsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(GetMaterialDescriptorSetInfo());
}
GameShaderSpecializationConstantFlag ShaderGameWorldLightingPass::GetStaticSpecializationConstantFlags(GameShaderSpecialization specialization) const
{
	auto staticFlags = GameShaderSpecializationConstantFlag::None;
	switch(specialization) {
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
std::optional<uint32_t> ShaderGameWorldLightingPass::FindPipelineIndex(rendering::PassType passType, GameShaderSpecialization specialization, GameShaderSpecializationConstantFlag specializationFlags) const
{
	if(GetContext().IsValidationEnabled())
		return 0; // We only have 1 pipeline if validation mode is enabled
	return ShaderSpecializationManager::FindSpecializationPipelineIndex(passType, GetStaticSpecializationConstantFlags(specialization) | specializationFlags);
}
static std::optional<Vector3> get_emission_factor(CMaterial &mat)
{
	auto &data = mat.GetDataBlock();
	auto &dbEmissionFactor = data->GetValue("emission_factor");
	if(dbEmissionFactor == nullptr || typeid(*dbEmissionFactor) != typeid(ds::Vector))
		return {};
	auto emissionFactor = static_cast<ds::Vector *>(dbEmissionFactor.get())->GetValue();
	emissionFactor *= data->GetFloat("emission_strength", 1.f);
	if(emissionFactor.r == 0.f && emissionFactor.g == 0.f && emissionFactor.b == 0.f)
		return {};
	return emissionFactor;
}
GameShaderSpecializationConstantFlag ShaderGameWorldLightingPass::GetMaterialPipelineSpecializationRequirements(CMaterial &mat) const
{
	auto flags = GameShaderSpecializationConstantFlag::None;
	auto hasEmission = (mat.GetTextureInfo(Material::EMISSION_MAP_IDENTIFIER) != nullptr);
	if(!hasEmission) {
		auto &data = mat.GetDataBlock();
		if(data->HasValue("emission_factor"))
			hasEmission = get_emission_factor(mat).has_value();
	}
	if(mat.GetAlphaMode() != AlphaMode::Opaque)
		flags |= GameShaderSpecializationConstantFlag::EnableTranslucencyBit;
	auto *rmaMap = mat.GetRMAMap();
	if(rmaMap) {
		auto texture = std::static_pointer_cast<Texture>(rmaMap->texture);
		if(texture) {
			auto texName = texture->GetName();
			ustring::to_lower(texName);
			auto path = util::Path::CreateFile(texName);
			path.RemoveFileExtension();
		}
	}
	return flags;
}
prosper::DescriptorSetInfo &ShaderGameWorldLightingPass::GetMaterialDescriptorSetInfo() const { return DESCRIPTOR_SET_MATERIAL; }
void ShaderGameWorldLightingPass::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}
void ShaderGameWorldLightingPass::InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_UV);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_NORMAL);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_TANGENT);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_BI_TANGENT);

	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_LIGHTMAP_UV);

	/*if(static_cast<Pipeline>(pipelineIdx) == Pipeline::LightMap)
	{
		AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_LIGHTMAP_UV);
		const auto lightMapEnabled = true;
		//pipelineInfo.add_specialization_constant(prosper::ShaderStage::FRAGMENT,0u,sizeof(lightMapEnabled),&lightMapEnabled);
		//pipelineInfo.add_specialization_constant(prosper::ShaderStage::VERTEX,0u,sizeof(lightMapEnabled),&lightMapEnabled);
	}*/
}
void ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, GetMaterialDescriptorSetInfo());
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_RENDERER);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_SHADOWS);
}
void ShaderGameWorldLightingPass::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderEntity::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	auto isReflection = (static_cast<rendering::PassType>(GetBasePassType(pipelineIdx)) == rendering::PassType::Reflection);
	if(isReflection) {
		prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo, prosper::CullModeFlags::FrontBit);
		//pipelineInfo.SetRasterizationProperties(prosper::PolygonMode::Line,prosper::CullModeFlags::FrontBit,prosper::FrontFace::Clockwise,1.f);
	}

	//uint32_t isSet = static_cast<uint32_t>(IsSpecializationConstantSet(pipelineIdx,flag));
	auto isTranslucentPipeline = IsSpecializationConstantSet(pipelineIdx, GameShaderSpecializationConstantFlag::EnableTranslucencyBit);
	if(isTranslucentPipeline)
		SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	pipelineInfo.ToggleDepthWrites(false); // Already written in depth pre-pass
	pipelineInfo.ToggleDepthTest(true, prosper::CompareOp::LessOrEqual);

	//pipelineInfo.ToggleDepthBias(true,0.f,0.f,0.f);
	//pipelineInfo.ToggleDynamicState(true,prosper::DynamicState::DepthBias); // Required for decals

	InitializeGfxPipelineVertexAttributes(pipelineInfo, pipelineIdx);
	InitializeGfxPipelinePushConstantRanges(pipelineInfo, pipelineIdx);
	InitializeGfxPipelineDescriptorSets(pipelineInfo, pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo, true);

	// Fragment
	ShaderSpecializationManager::AddSpecializationConstant(*this, pipelineInfo, pipelineIdx, prosper::ShaderStageFlags::FragmentBit, GameShaderSpecializationConstantFlag::EnableTranslucencyBit);

	// Shared
	ShaderSpecializationManager::AddSpecializationConstant(*this, pipelineInfo, pipelineIdx, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit, GameShaderSpecializationConstantFlag::EnableLightMapsBit);

	// Vertex
	ShaderSpecializationManager::AddSpecializationConstant(*this, pipelineInfo, pipelineIdx, prosper::ShaderStageFlags::VertexBit, GameShaderSpecializationConstantFlag::EnableAnimationBit);
	ShaderSpecializationManager::AddSpecializationConstant(*this, pipelineInfo, pipelineIdx, prosper::ShaderStageFlags::VertexBit, GameShaderSpecializationConstantFlag::EnableMorphTargetAnimationBit);

	// Properties
	auto &shaderSettings = client->GetGameWorldShaderSettings();
	auto fSetPropertyValue = [this, &pipelineInfo](GameShaderSpecializationPropertyIndex prop, auto value) { ShaderGameWorld::AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, umath::to_integral(prop), sizeof(value), &value); };
	fSetPropertyValue(GameShaderSpecializationPropertyIndex::ShadowQuality, shaderSettings.shadowQuality);
	fSetPropertyValue(GameShaderSpecializationPropertyIndex::DebugModeEnabled, static_cast<uint32_t>(shaderSettings.debugModeEnabled));
	fSetPropertyValue(GameShaderSpecializationPropertyIndex::BloomOutputEnabled, static_cast<uint32_t>(shaderSettings.bloomEnabled));
	fSetPropertyValue(GameShaderSpecializationPropertyIndex::EnableSsao, static_cast<uint32_t>(shaderSettings.ssaoEnabled));
	fSetPropertyValue(GameShaderSpecializationPropertyIndex::EnableIbl, static_cast<uint32_t>(shaderSettings.iblEnabled));
	fSetPropertyValue(GameShaderSpecializationPropertyIndex::EnableDynamicLighting, static_cast<uint32_t>(shaderSettings.dynamicLightingEnabled));
	fSetPropertyValue(GameShaderSpecializationPropertyIndex::EnableDynamicShadows, static_cast<uint32_t>(shaderSettings.dynamicShadowsEnabled));
}

std::shared_ptr<Texture> ShaderGameWorldLightingPass::GetTexture(const std::string &texName)
{
	auto &matManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	auto *asset = texManager.FindCachedAsset(texName);
	if(!asset)
		return nullptr;
	auto ptrTex = msys::TextureManager::GetAssetObject(*asset);
	if(ptrTex == nullptr)
		return nullptr;
	return std::static_pointer_cast<Texture>(ptrTex);
}

static void to_srgb_color(Vector4 &col) { col = Vector4 {uimg::linear_to_srgb(reinterpret_cast<Vector3 &>(col)), col.w}; }

static auto cvNormalMappingEnabled = GetClientConVar("render_normalmapping_enabled");
void ShaderGameWorldLightingPass::ApplyMaterialFlags(CMaterial &mat, MaterialFlags &outFlags) const {}
ShaderGameWorldLightingPass::MaterialData ShaderGameWorldLightingPass::GenerateMaterialData(CMaterial &mat)
{
	MaterialData matData {};

	auto &matFlags = matData.flags;

	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap && diffuseMap->texture && std::static_pointer_cast<Texture>(diffuseMap->texture)->HasFlag(Texture::Flags::SRGB))
		matFlags |= MaterialFlags::DiffuseSRGB;

	auto &data = mat.GetDataBlock();
	if(data == nullptr)
		return {};
	auto *parallaxMap = mat.GetParallaxMap();
	if(parallaxMap != nullptr && parallaxMap->texture != nullptr) {
		matFlags |= MaterialFlags::Parallax;

		float heightScale = matData.GetParallaxHeightScale();
		data->GetFloat("parallax_height_scale", &heightScale);
		matData.SetParallaxHeightScale(heightScale);
		int32_t parallaxSteps = matData.parallaxSteps;
		data->GetInt("parallax_steps", &parallaxSteps);
		matData.parallaxSteps = parallaxSteps;
	}

	if(cvNormalMappingEnabled->GetBool() == true) {
		auto *normalMap = mat.GetNormalMap();
		if(normalMap != nullptr && normalMap->texture != nullptr)
			matFlags |= MaterialFlags::Normal;
	}

	if(data->GetBool("black_to_alpha") == true)
		matFlags |= MaterialFlags::BlackToAlpha;

	matData.color = {1.f, 1.f, 1.f, 1.f};
	data->GetVector3("color_factor", reinterpret_cast<Vector3 *>(&matData.color));
	data->GetFloat("alpha_factor", &matData.color.a);
	to_srgb_color(matData.color);

	if(data->GetBool("debug_mode", false))
		matFlags |= MaterialFlags::Debug;

	auto *glowMap = mat.GetGlowMap();
	auto hasGlowmap = (glowMap != nullptr && glowMap->texture != nullptr);
	if(hasGlowmap || data->HasValue("emission_factor")) {
		auto emissionFactor = get_emission_factor(mat);
		if(emissionFactor) {
			matData.emissionFactor = {emissionFactor->r, emissionFactor->g, emissionFactor->b, 1.f};
			to_srgb_color(matData.emissionFactor);
		}

		if(hasGlowmap || emissionFactor) {
			std::shared_ptr<Texture> texture;
			if(hasGlowmap)
				texture = std::static_pointer_cast<Texture>(glowMap->texture);
			if(emissionFactor) {
				matFlags |= MaterialFlags::GlowSRGB;
				if(!texture) {
					texture = GetTexture("white");
					hasGlowmap = true;
				}
			}
			auto bUseGlow = true;
			if(hasGlowmap) {
				if(texture->HasFlag(Texture::Flags::SRGB))
					matFlags |= MaterialFlags::GlowSRGB;
				if(data->GetBool("glow_alpha_only") == true) {
					if(prosper::util::has_alpha(texture->GetVkTexture()->GetImage().GetFormat()) == false)
						bUseGlow = false;
				}
			}
			if(bUseGlow == true) {
				int32_t glowMode = 1;
				data->GetInt("glow_blend_diffuse_mode", &glowMode);
				if(glowMode != 0) {
					matFlags |= MaterialFlags::Glow;
					data->GetFloat("glow_blend_diffuse_scale", &matData.glowScale);
				}
				switch(glowMode) {
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
	}

	data->GetFloat("alpha_discard_threshold", &matData.alphaDiscardThreshold);

	auto hasRmaMap = (data->GetValue(Material::RMA_MAP_IDENTIFIER) != nullptr);
	auto defaultMetalness = hasRmaMap ? 1.f : 0.f;
	auto defaultRoughness = hasRmaMap ? 1.f : 0.5f;
	auto defaultAo = hasRmaMap ? 1.f : 0.f;

	matData.metalnessFactor = defaultMetalness;
	data->GetFloat("metalness_factor", &matData.metalnessFactor);

	matData.roughnessFactor = defaultRoughness;
	auto hasRoughnessFactor = data->GetFloat("roughness_factor", &matData.roughnessFactor);

	matData.aoFactor = defaultAo;
	data->GetFloat("ao_factor", &matData.aoFactor);

	float specularFactor;
	if(data->GetFloat("specular_factor", &specularFactor)) {
		if(hasRoughnessFactor == false)
			matData.roughnessFactor = 1.f;
		matData.roughnessFactor *= (1.f - specularFactor);
	}

	auto alphaMode = AlphaMode::Opaque;
	if(data->IsString("alpha_mode")) {
		auto e = magic_enum::enum_cast<AlphaMode>(data->GetString("alpha_mode"));
		alphaMode = e.has_value() ? *e : AlphaMode::Opaque;
	}
	else
		alphaMode = static_cast<AlphaMode>(data->GetInt("alpha_mode", umath::to_integral(AlphaMode::Opaque)));
	matData.alphaMode = alphaMode;

	auto alphaCutoff = 0.5f;
	data->GetFloat("alpha_cutoff", &alphaCutoff);
	matData.alphaCutoff = alphaCutoff;

	if(mat.GetAlphaMode() != AlphaMode::Opaque)
		matFlags |= MaterialFlags::Translucent;
	if(mat.GetTextureInfo(Material::WRINKLE_STRETCH_MAP_IDENTIFIER) || mat.GetTextureInfo(Material::WRINKLE_COMPRESS_MAP_IDENTIFIER))
		matFlags |= MaterialFlags::WrinkleMaps;

	auto *rmaMap = mat.GetRMAMap();
	if(rmaMap) {
		matFlags |= MaterialFlags::RmaMap;
		auto texture = std::static_pointer_cast<Texture>(rmaMap->texture);
		if(texture) {
			auto texName = texture->GetName();
			ustring::to_lower(texName);
			auto path = util::Path::CreateFile(texName);
			path.RemoveFileExtension();
			if(path == "pbr/rma_neutral") {
				// If the material uses the neutral rma texture, we can completely ignore
				// it for the shader and use the default values instead, which saves
				// a lot of texture lookups.
				matFlags &= ~MaterialFlags::RmaMap;
			}
		}
	}

	return matData;
}
std::optional<ShaderGameWorldLightingPass::MaterialData> ShaderGameWorldLightingPass::UpdateMaterialBuffer(CMaterial &mat)
{
	auto *buf = mat.GetSettingsBuffer();
	if(buf == nullptr)
		return {};

	auto matData = GenerateMaterialData(mat);
	ApplyMaterialFlags(mat, matData.flags);
	InitializeMaterialData(mat, matData);
	buf->Write(0, matData);
	return matData;
}
void ShaderGameWorldLightingPass::UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags) {}
bool ShaderGameWorldLightingPass::IsDepthPrepassEnabled() const { return m_depthPrepassEnabled; }
uint32_t ShaderGameWorldLightingPass::GetCameraDescriptorSetIndex() const { return DESCRIPTOR_SET_SCENE.setIndex; }
uint32_t ShaderGameWorldLightingPass::GetRendererDescriptorSetIndex() const { return DESCRIPTOR_SET_RENDERER.setIndex; }
uint32_t ShaderGameWorldLightingPass::GetInstanceDescriptorSetIndex() const { return DESCRIPTOR_SET_INSTANCE.setIndex; }
uint32_t ShaderGameWorldLightingPass::GetRenderSettingsDescriptorSetIndex() const { return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex; }
uint32_t ShaderGameWorldLightingPass::GetLightDescriptorSetIndex() const { return DESCRIPTOR_SET_LIGHTS.setIndex; }
uint32_t ShaderGameWorldLightingPass::GetMaterialDescriptorSetIndex() const { return GetMaterialDescriptorSetInfo().setIndex; }
void ShaderGameWorldLightingPass::InitializeMaterialData(CMaterial &mat, MaterialData &matData) {}
void ShaderGameWorldLightingPass::GetVertexAnimationPushConstantInfo(uint32_t &offset) const { offset = offsetof(PushConstants, vertexAnimInfo); }
bool ShaderGameWorldLightingPass::GetRenderBufferTargets(CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const
{
	if(ShaderEntity::GetRenderBufferTargets(mesh, pipelineIdx, outBuffers, outOffsets, outIndexBufferInfo) == false)
		return false;
	auto &sceneMesh = mesh.GetSceneMesh();
	outBuffers.push_back(nullptr); // Lightmap uv buffer is instance-based and handled by the model entity component
	outOffsets.push_back(0ull);
	return true;
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo)
{
	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap == nullptr || diffuseMap->texture == nullptr) {
		spdlog::debug("Failed to initialize material descriptor set for material '{}': Material has no albedo map!", mat.GetName());
		return nullptr;
	}
	auto diffuseTexture = std::static_pointer_cast<Texture>(diffuseMap->texture);
	if(diffuseTexture->HasValidVkTexture() == false) {
		spdlog::debug("Failed to initialize material descriptor set for material '{}': Albedo map of material is not valid!", mat.GetName());
		return nullptr;
	}
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(descSetInfo);
	mat.SetDescriptorSetGroup(*this, descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*diffuseTexture->GetVkTexture(), umath::to_integral(MaterialBinding::DiffuseMap));

	auto *normalMap = mat.GetNormalMap();
	if(normalMap != nullptr && normalMap->texture != nullptr) {
		auto texture = std::static_pointer_cast<Texture>(normalMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(), umath::to_integral(MaterialBinding::NormalMap));
	}

	auto *parallaxMap = mat.GetParallaxMap();
	if(parallaxMap != nullptr && parallaxMap->texture != nullptr) {
		auto texture = std::static_pointer_cast<Texture>(parallaxMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(), umath::to_integral(MaterialBinding::ParallaxMap));
	}

	auto *glowMap = mat.GetGlowMap();
	if(glowMap != nullptr && glowMap->texture != nullptr) {
		auto texture = std::static_pointer_cast<Texture>(glowMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(), umath::to_integral(MaterialBinding::GlowMap));
	}
	InitializeMaterialBuffer(descSet, mat);

	return descSetGroup;
}
std::optional<ShaderGameWorldLightingPass::MaterialData> ShaderGameWorldLightingPass::InitializeMaterialBuffer(prosper::IDescriptorSet &descSet, CMaterial &mat, uint32_t bindingIdx)
{
	auto settingsBuffer = mat.GetSettingsBuffer() ? mat.GetSettingsBuffer()->shared_from_this() : nullptr;
	if(settingsBuffer == nullptr && g_materialSettingsBuffer)
		settingsBuffer = g_materialSettingsBuffer->AllocateBuffer();
	if(settingsBuffer == nullptr)
		return {};
	descSet.SetBindingUniformBuffer(*settingsBuffer, bindingIdx);
	mat.SetSettingsBuffer(*settingsBuffer);

	auto matData = GenerateMaterialData(mat);

	settingsBuffer->Write(0, matData);
	return matData;
}
std::optional<ShaderGameWorldLightingPass::MaterialData> ShaderGameWorldLightingPass::InitializeMaterialBuffer(prosper::IDescriptorSet &descSet, CMaterial &mat)
{
	auto settingsBuffer = mat.GetSettingsBuffer() ? mat.GetSettingsBuffer()->shared_from_this() : nullptr;
	if(settingsBuffer == nullptr && g_materialSettingsBuffer)
		settingsBuffer = g_materialSettingsBuffer->AllocateBuffer();
	if(settingsBuffer == nullptr)
		return {};
	descSet.SetBindingUniformBuffer(*settingsBuffer, umath::to_integral(MaterialBinding::MaterialSettings));
	mat.SetSettingsBuffer(*settingsBuffer);
	return UpdateMaterialBuffer(mat);
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(CMaterial &mat) { return InitializeMaterialDescriptorSet(mat, DESCRIPTOR_SET_MATERIAL); }

////////

GameShaderSpecializationConstantFlag ShaderGameWorldLightingPass::GetBaseSpecializationFlags() const { return GameShaderSpecializationConstantFlag::None; }

void ShaderGameWorldLightingPass::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, prosper::IDescriptorSet &dsMaterial, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	std::array<prosper::IDescriptorSet *, 6> descSets {descSets[0] = &dsMaterial, descSets[1] = &dsScene, descSets[2] = &dsRenderer, descSets[3] = &dsRenderSettings, descSets[4] = &dsLights, descSets[5] = &dsShadows};

	RecordPushSceneConstants(shaderProcessor, scene, drawOrigin);
	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), pragma::ShaderGameWorld::MATERIAL_DESCRIPTOR_SET_INDEX, descSets, dynamicOffsets);
}

bool ShaderGameWorldLightingPass::RecordPushSceneConstants(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const Vector4 &drawOrigin) const
{
	ShaderGameWorldLightingPass::PushConstants pushConstants {};
	pushConstants.Initialize();
	pushConstants.drawOrigin = drawOrigin;
	auto &hCam = scene.GetActiveCamera();
	assert(hCam.valid());
	pushConstants.debugMode = scene.GetDebugMode();
	pushConstants.flags = m_sceneFlags;
	return shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, 0u, sizeof(pushConstants), &pushConstants);
}

////////

uint32_t ShaderSpecializationManager::GetPipelineIndexStartOffset(PassTypeIndex passType) const
{
	if(passType >= m_passTypeSpecializationToPipelineIdx.size())
		return 0;
	return m_passTypeSpecializationToPipelineIdx[passType].pipelineIndexRange.first;
}
void ShaderSpecializationManager::SetPipelineIndexRange(PassTypeIndex passType, uint32_t startIndex, uint32_t endIndex)
{
	auto &info = InitializePassTypeSpecializations(passType);
	info.pipelineIndexRange = {startIndex, endIndex};
}
ShaderSpecializationManager::PassTypeIndex ShaderSpecializationManager::GetBasePassType(uint32_t pipelineIdx) const
{
	for(auto it = m_passTypeSpecializationToPipelineIdx.begin(); it != m_passTypeSpecializationToPipelineIdx.end(); ++it) {
		auto &info = *it;
		if(pipelineIdx >= info.pipelineIndexRange.first && pipelineIdx < info.pipelineIndexRange.second)
			return it - m_passTypeSpecializationToPipelineIdx.begin();
	}
	throw std::logic_error {"Invalid pipeline index " + std::to_string(pipelineIdx) + "!"};
}
bool ShaderSpecializationManager::IsSpecializationConstantSet(uint32_t pipelineIdx, SpecializationFlags flag) const
{
	if(pipelineIdx >= m_pipelineSpecializations.size())
		return false;
	auto flags = m_pipelineSpecializations[pipelineIdx];
	return (flags & flag) != 0;
}
ShaderSpecializationManager::PassTypeInfo &ShaderSpecializationManager::InitializePassTypeSpecializations(PassTypeIndex passType)
{
	if(passType >= m_passTypeSpecializationToPipelineIdx.size()) {
		auto n = m_passTypeSpecializationToPipelineIdx.size();
		m_passTypeSpecializationToPipelineIdx.resize(passType + 1);
		for(auto i = n; i < m_passTypeSpecializationToPipelineIdx.size(); ++i)
			std::fill(m_passTypeSpecializationToPipelineIdx[i].specializationToPipelineIdx.begin(), m_passTypeSpecializationToPipelineIdx[i].specializationToPipelineIdx.end(), std::numeric_limits<uint32_t>::max());
	}
	return m_passTypeSpecializationToPipelineIdx[passType];
}
void ShaderSpecializationManager::RegisterSpecializations(PassTypeIndex passType, SpecializationFlags staticFlags, SpecializationFlags dynamicFlags)
{
	auto &specializationMap = InitializePassTypeSpecializations(passType).specializationToPipelineIdx;
	auto dynamicFlagValues = umath::get_power_of_2_values(dynamicFlags);
	auto &permutations = m_pipelineSpecializations;
	std::function<void(uint32_t, SpecializationFlags)> registerSpecialization = nullptr;
	permutations.reserve(permutations.size() + umath::pow(static_cast<size_t>(2), dynamicFlagValues.size()));
	registerSpecialization = [&specializationMap, &registerSpecialization, &dynamicFlagValues, &permutations](uint32_t idx, SpecializationFlags perm) {
		if(idx >= dynamicFlagValues.size()) {
			permutations.push_back(perm);
			specializationMap[perm] = permutations.size() - 1;
			return;
		}
		auto curFlag = dynamicFlagValues[idx];
		registerSpecialization(idx + 1, perm);
		registerSpecialization(idx + 1, perm |= curFlag);
	};
	registerSpecialization(0, staticFlags);
}
std::optional<uint32_t> ShaderSpecializationManager::FindSpecializationPipelineIndex(PassTypeIndex passType, uint64_t specializationFlags) const
{
	if(passType >= m_passTypeSpecializationToPipelineIdx.size())
		return {};
	auto &specToPipelineIdx = m_passTypeSpecializationToPipelineIdx[passType].specializationToPipelineIdx;
	auto pipelineIdx = specToPipelineIdx[specializationFlags];
	return (pipelineIdx != std::numeric_limits<uint32_t>::max()) ? pipelineIdx : std::optional<uint32_t> {};
}
