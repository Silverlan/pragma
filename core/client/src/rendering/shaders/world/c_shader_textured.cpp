/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/model/vk_mesh.h"
#include "pragma/console/c_cvar_global_functions.h"
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
#include <pragma/asset/util_asset.hpp>
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
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_SCENE) ShaderGameWorldLightingPass::DESCRIPTOR_SET_SCENE = {&ShaderEntity::DESCRIPTOR_SET_SCENE};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDERER) ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDERER = {&ShaderEntity::DESCRIPTOR_SET_RENDERER};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderEntity::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_LIGHTS) ShaderGameWorldLightingPass::DESCRIPTOR_SET_LIGHTS = {&ShaderEntity::DESCRIPTOR_SET_LIGHTS};
decltype(ShaderGameWorldLightingPass::DESCRIPTOR_SET_SHADOWS) ShaderGameWorldLightingPass::DESCRIPTOR_SET_SHADOWS = {&ShaderEntity::DESCRIPTOR_SET_SHADOWS};

static std::shared_ptr<prosper::IUniformResizableBuffer> g_materialSettingsBuffer = nullptr;
static uint32_t g_instanceCount = 0;
static void initialize_material_settings_cache()
{
	if(g_materialSettingsBuffer)
		return;
	// Note: Using a uniform resizable buffer for this doesn't work, because the buffers are used by
	// descriptor sets, which would have to be updated whenever the buffer is re-allocated (which currently
	// does not happen automatically). TODO: Implement this? On the other hand, material data
	// isn't that big to begin with, so maybe just make sure the buffer is large enough for all use cases?
	constexpr auto matSize = pragma::rendering::shader_material::MAX_MATERIAL_SIZE;
	constexpr size_t count = 524'288;
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	//bufCreateInfo.size = sizeof(ShaderGameWorldLightingPass::MaterialData) *2'048;
	bufCreateInfo.size = matSize * count; // ~64 MiB
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit | prosper::BufferUsageFlags::UniformBufferBit;
	bufCreateInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	g_materialSettingsBuffer = c_engine->GetRenderContext().CreateUniformResizableBuffer(bufCreateInfo, matSize, matSize * count, 0.05f);
	g_materialSettingsBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);
}
ShaderGameWorldLightingPass::ShaderGameWorldLightingPass(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderGameWorld(context, identifier, vsShader, fsShader, gsShader)
{
	if(g_instanceCount++ == 0u)
		initialize_material_settings_cache();

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
	if(--g_instanceCount == 0) {
		g_materialSettingsBuffer = nullptr;
		pragma::rendering::shader_material ::clear_cache();
	}
}
void ShaderGameWorldLightingPass::GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode)
{
	ShaderGameWorld::GetShaderPreprocessorDefinitions(outDefinitions, outPrefixCode);
	if(m_shaderMaterial)
		outPrefixCode += m_shaderMaterial->ToGlslStruct();
}
uint32_t ShaderGameWorldLightingPass::GetPassPipelineIndexStartOffset(rendering::PassType passType) const { return GetPipelineIndexStartOffset(umath::to_integral(passType)); }
void ShaderGameWorldLightingPass::OnPipelinesInitialized() { ShaderGameWorld::OnPipelinesInitialized(); }
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
void ShaderGameWorldLightingPass::SetShaderMaterialName(const std::optional<std::string> &shaderMaterial) { m_shaderMaterialName = shaderMaterial; }
const std::optional<std::string> &ShaderGameWorldLightingPass::GetShaderMaterialName() const { return m_shaderMaterialName; }
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
prosper::DescriptorSetInfo &ShaderGameWorldLightingPass::GetMaterialDescriptorSetInfo() const { return *m_materialDescSetInfo; }
void ShaderGameWorldLightingPass::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }
void ShaderGameWorldLightingPass::InitializeGfxPipelineVertexAttributes()
{
	AddVertexAttribute(VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(VERTEX_ATTRIBUTE_UV);
	AddVertexAttribute(VERTEX_ATTRIBUTE_NORMAL);
	AddVertexAttribute(VERTEX_ATTRIBUTE_TANGENT);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BI_TANGENT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_LIGHTMAP_UV);

	/*if(static_cast<Pipeline>(pipelineIdx) == Pipeline::LightMap)
	{
		AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_LIGHTMAP_UV);
		const auto lightMapEnabled = true;
		//pipelineInfo.add_specialization_constant(prosper::ShaderStage::FRAGMENT,0u,sizeof(lightMapEnabled),&lightMapEnabled);
		//pipelineInfo.add_specialization_constant(prosper::ShaderStage::VERTEX,0u,sizeof(lightMapEnabled),&lightMapEnabled);
	}*/
}
uint32_t ShaderGameWorldLightingPass::GetSceneDescriptorSetIndex() const { return DESCRIPTOR_SET_SCENE.setIndex; };
void ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets()
{
	if(!m_materialDescSetInfo)
		throw std::runtime_error {"Material descriptor set info is not valid!"};
	AddDescriptorSetGroup(DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(*m_materialDescSetInfo);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_RENDERER);
	AddDescriptorSetGroup(DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SHADOWS);
}
std::unique_ptr<prosper::DescriptorSetInfo> ShaderGameWorldLightingPass::CreateMaterialDescriptorSetInfo(const pragma::rendering::shader_material::ShaderMaterial &shaderMaterial)
{
	std::vector<prosper::DescriptorSetInfo::Binding> bindings;
	bindings.reserve(shaderMaterial.textures.size() + 1);

	size_t bindingIdx = 0;
	{
		prosper::DescriptorSetInfo::Binding binding {};
		binding.bindingIndex = bindingIdx++;
		binding.name = "SETTINGS";
		binding.shaderStages = prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::GeometryBit;
		binding.type = prosper::DescriptorType::UniformBuffer;
		binding.flags = prosper::PrDescriptorSetBindingFlags::None;
		bindings.push_back(binding);
	}

	for(auto &tex : shaderMaterial.textures) {
		prosper::DescriptorSetInfo::Binding binding {};
		binding.bindingIndex = bindingIdx++;
		binding.name = tex.name.str;
		binding.shaderStages = prosper::ShaderStageFlags::FragmentBit;
		binding.type = prosper::DescriptorType::CombinedImageSampler;
		binding.flags = prosper::PrDescriptorSetBindingFlags::None;
		if(tex.cubemap)
			binding.flags |= prosper::PrDescriptorSetBindingFlags::Cubemap;
		bindings.push_back(binding);
	}
	return std::make_unique<prosper::DescriptorSetInfo>("MATERIAL", bindings);
}
void ShaderGameWorldLightingPass::InitializeShaderMaterial()
{
	if(m_shaderMaterialName) {
		m_shaderMaterial = pragma::rendering::shader_material::get_cache().Load(*m_shaderMaterialName);
		m_materialDescSetInfo = CreateMaterialDescriptorSetInfo(*m_shaderMaterial);
	}
	else
		throw std::runtime_error {"Invalid shader material for shader '" + GetIdentifier() + "'!"};
}
void ShaderGameWorldLightingPass::InitializeShaderResources()
{
	InitializeShaderMaterial();

	ShaderEntity::InitializeShaderResources();
	InitializeGfxPipelineVertexAttributes();
	InitializeGfxPipelinePushConstantRanges();
	InitializeGfxPipelineDescriptorSets();
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

std::shared_ptr<Texture> ShaderGameWorldLightingPass::GetTexture(const std::string &texName, bool load)
{
	auto &matManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	auto *asset = texManager.FindCachedAsset(texName);
	if(load && !asset) {
		texManager.LoadAsset(texName);
		asset = texManager.FindCachedAsset(texName);
	}
	if(!asset)
		return nullptr;
	auto ptrTex = msys::TextureManager::GetAssetObject(*asset);
	if(ptrTex == nullptr)
		return nullptr;
	return std::static_pointer_cast<Texture>(ptrTex);
}

static auto cvNormalMappingEnabled = GetClientConVar("render_normalmapping_enabled");
void ShaderGameWorldLightingPass::ApplyMaterialFlags(CMaterial &mat, rendering::shader_material::MaterialFlags &outFlags) const {}
void ShaderGameWorldLightingPass::UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags) {}
bool ShaderGameWorldLightingPass::IsDepthPrepassEnabled() const { return m_depthPrepassEnabled; }
uint32_t ShaderGameWorldLightingPass::GetCameraDescriptorSetIndex() const { return DESCRIPTOR_SET_SCENE.setIndex; }
uint32_t ShaderGameWorldLightingPass::GetRendererDescriptorSetIndex() const { return DESCRIPTOR_SET_RENDERER.setIndex; }
uint32_t ShaderGameWorldLightingPass::GetInstanceDescriptorSetIndex() const { return DESCRIPTOR_SET_INSTANCE.setIndex; }
uint32_t ShaderGameWorldLightingPass::GetRenderSettingsDescriptorSetIndex() const { return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex; }
uint32_t ShaderGameWorldLightingPass::GetLightDescriptorSetIndex() const { return DESCRIPTOR_SET_LIGHTS.setIndex; }
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
	if(!m_shaderMaterial)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(descSetInfo);
	mat.SetDescriptorSetGroup(*this, descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	auto materialFlags = pragma::rendering::shader_material::MaterialFlags::None;
	size_t textureBinding = 1; // First binding is for material settings
	uint32_t isrgb = 0;
	for(auto &shaderTexInfo : m_shaderMaterial->textures) {
		std::shared_ptr<prosper::Texture> tex {};
		std::shared_ptr<Texture> texData {};
		auto *texInfo = mat.GetTextureInfo(shaderTexInfo.name);
		if(texInfo != nullptr && texInfo->texture != nullptr) {
			texData = std::static_pointer_cast<Texture>(texInfo->texture);
			if(texData && texData->HasValidVkTexture())
				tex = texData->GetVkTexture();
		}
		if(!tex && shaderTexInfo.defaultTexturePath) {
			texData = pragma::ShaderGameWorldLightingPass::GetTexture(*shaderTexInfo.defaultTexturePath, true);
			if(texData && texData->HasValidVkTexture())
				tex = texData->GetVkTexture();
			else {
				spdlog::error("Failed to bind default texture '{}' to texture binding '{}'!", *shaderTexInfo.defaultTexturePath, shaderTexInfo.name.str);
				return nullptr;
			}
		}
		if(!tex) {
			if(shaderTexInfo.required) {
				spdlog::error("Failed to bind texture '{}' to texture binding '{}'!", (texData ? texData->GetName() : "NULL"), shaderTexInfo.name.str);
				return nullptr;
			}
			++textureBinding;
			continue;
		}
		descSet.SetBindingTexture(*tex, textureBinding);

		if(shaderTexInfo.colorMap) {
			if(isrgb >= pragma::rendering::shader_material::MAX_NUMBER_OF_SRGB_TEXTURES) {
				spdlog::error("Number of SRGB textures exceeds maximum limit of {}!", pragma::rendering::shader_material::MAX_NUMBER_OF_SRGB_TEXTURES);
				return nullptr;
			}
			if(prosper::util::is_srgb_format(tex->GetImage().GetFormat()))
				materialFlags |= static_cast<pragma::rendering::shader_material::MaterialFlags>(umath::to_integral(pragma::rendering::shader_material::MaterialFlags::Srgb0) + isrgb);
			++isrgb;
		}

		using namespace ustring::string_switch_ci;
		switch(hash(shaderTexInfo.name)) {
		case "normal_map"_:
			materialFlags |= pragma::rendering::shader_material::MaterialFlags::HasNormalMap;
			break;
		case "parallax_map"_:
			materialFlags |= pragma::rendering::shader_material::MaterialFlags::HasParallaxMap;
			break;
		case "emission_map"_:
			materialFlags |= pragma::rendering::shader_material::MaterialFlags::HasEmissionMap;
			break;
		case "rma_map"_:
			{
				materialFlags |= pragma::rendering::shader_material::MaterialFlags::HasRmaMap;
				auto path = pragma::asset::get_normalized_path(texData->GetName(), pragma::asset::Type::Texture);
				static const auto rmaNeutralPath = pragma::asset::get_normalized_path("pbr/rma_neutral", pragma::asset::Type::Texture);
				if(path == rmaNeutralPath) {
					// If the material uses the neutral rma texture, we can completely ignore
					// it for the shader and use the default values instead, which saves
					// a lot of texture lookups.
					materialFlags &= ~pragma::rendering::shader_material::MaterialFlags::HasRmaMap;
				}
				break;
			}
		case "wrinkle_compress_map"_:
		case "wrinkle_stretch_map"_:
			materialFlags |= pragma::rendering::shader_material::MaterialFlags::HasWrinkleMaps;
			break;
		}

		++textureBinding;
	}

	pragma::rendering::shader_material::ShaderMaterialData materialData {*m_shaderMaterial};
	materialData.PopulateFromMaterial(mat);
	InitializeMaterialData(mat, *m_shaderMaterial, materialData);

	materialFlags |= materialData.GetFlags();
	auto alphaMode = materialData.GetValue<uint32_t>("alpha_mode");
	if(alphaMode && static_cast<AlphaMode>(*alphaMode) != AlphaMode::Opaque)
		materialFlags |= pragma::rendering::shader_material::MaterialFlags::Translucent;

	materialData.SetFlags(materialFlags);
	InitializeMaterialBuffer(descSet, mat, materialData);

	return descSetGroup;
}
bool ShaderGameWorldLightingPass::InitializeMaterialBuffer(prosper::IDescriptorSet &descSet, CMaterial &mat, const pragma::rendering::shader_material::ShaderMaterialData &matData, uint32_t bindingIdx)
{
	auto settingsBuffer = mat.GetSettingsBuffer() ? mat.GetSettingsBuffer()->shared_from_this() : nullptr;
	if(settingsBuffer == nullptr && g_materialSettingsBuffer)
		settingsBuffer = g_materialSettingsBuffer->AllocateBuffer();
	if(settingsBuffer == nullptr)
		return false;
	descSet.SetBindingUniformBuffer(*settingsBuffer, bindingIdx);
	mat.SetSettingsBuffer(*settingsBuffer);
	return settingsBuffer->Write(0, matData.data.size(), matData.data.data());
}
void ShaderGameWorldLightingPass::InitializeMaterialData(const CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::shader_material::ShaderMaterialData &inOutMatData) {}
bool ShaderGameWorldLightingPass::InitializeMaterialBuffer(prosper::IDescriptorSet &descSet, CMaterial &mat, const pragma::rendering::shader_material::ShaderMaterialData &matData)
{
	return InitializeMaterialBuffer(descSet, mat, matData, umath::to_integral(MaterialBinding::MaterialSettings));
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(CMaterial &mat) { return InitializeMaterialDescriptorSet(mat, GetMaterialDescriptorSetInfo()); }

////////

GameShaderSpecializationConstantFlag ShaderGameWorldLightingPass::GetBaseSpecializationFlags() const { return GameShaderSpecializationConstantFlag::None; }

void ShaderGameWorldLightingPass::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	std::array<prosper::IDescriptorSet *, 5> descSets {descSets[1] = &dsScene, descSets[2] = &dsRenderer, descSets[3] = &dsRenderSettings, descSets[4] = &dsLights, descSets[5] = &dsShadows};

	RecordPushSceneConstants(shaderProcessor, scene, drawOrigin);
	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetSceneDescriptorSetIndex(), descSets, dynamicOffsets);
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

static void print_shader_material_data(CMaterial &mat)
{
	auto *shader = dynamic_cast<ShaderGameWorldLightingPass *>(mat.GetPrimaryShader());
	if(!shader) {
		Con::cwar << "Material '" << mat.GetName() << "' has no primary shader!" << Con::endl;
		return;
	}
	auto *shaderMat = shader->GetShaderMaterial();
	if(!shaderMat) {
		Con::cwar << "Shader '" << shader->GetIdentifier() << "' has no shader material!" << Con::endl;
		return;
	}
	auto *buf = mat.GetSettingsBuffer();
	if(!buf) {
		Con::cwar << "Material '" << mat.GetName() << "' has no settings buffer!" << Con::endl;
		return;
	}
	pragma::rendering::shader_material::ShaderMaterialData shaderMatData {*shaderMat};
	if(!buf->Read(0, shaderMatData.data.size(), shaderMatData.data.data())) {
		Con::cwar << "Failed to read settings buffer data of material '" << mat.GetName() << "'!" << Con::endl;
		return;
	}
	shaderMatData.DebugPrint();
}

void Console::commands::debug_print_shader_material_data(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::cwar << "No material specified!" << Con::endl;
		return;
	}
	auto &matName = argv.front();
	auto *mat = client->LoadMaterial(matName);
	if(!mat) {
		Con::cwar << "Failed to load material '" << matName << "'!" << Con::endl;
		return;
	}
	print_shader_material_data(static_cast<CMaterial &>(*mat));
}