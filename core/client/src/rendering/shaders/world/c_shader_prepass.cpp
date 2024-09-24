/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

decltype(ShaderPrepassBase::VERTEX_BINDING_RENDER_BUFFER_INDEX) ShaderPrepassBase::VERTEX_BINDING_RENDER_BUFFER_INDEX = {prosper::VertexInputRate::Instance};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX) ShaderPrepassBase::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX = {ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX, VERTEX_BINDING_RENDER_BUFFER_INDEX};

decltype(ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT) ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID, VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT, VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderPrepassBase::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID, VERTEX_BINDING_BONE_WEIGHT_EXT};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderPrepassBase::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT, VERTEX_BINDING_BONE_WEIGHT_EXT};

decltype(ShaderPrepassBase::VERTEX_BINDING_VERTEX) ShaderPrepassBase::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex, sizeof(VertexBufferData)};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_POSITION) ShaderPrepassBase::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION, VERTEX_BINDING_VERTEX};
decltype(ShaderPrepassBase::VERTEX_ATTRIBUTE_UV) ShaderPrepassBase::VERTEX_ATTRIBUTE_UV = {ShaderEntity::VERTEX_ATTRIBUTE_UV, VERTEX_BINDING_VERTEX};

decltype(ShaderPrepassBase::DESCRIPTOR_SET_INSTANCE) ShaderPrepassBase::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderPrepassBase::DESCRIPTOR_SET_SCENE) ShaderPrepassBase::DESCRIPTOR_SET_SCENE = {&ShaderScene::DESCRIPTOR_SET_SCENE};
decltype(ShaderPrepassBase::DESCRIPTOR_SET_MATERIAL) ShaderPrepassBase::DESCRIPTOR_SET_MATERIAL = {
  "TEXTURES",
  {prosper::DescriptorSetInfo::Binding {"ALBEDO", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderPrepassBase::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderPrepassBase::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS};

prosper::util::RenderPassCreateInfo::AttachmentInfo ShaderPrepassBase::get_depth_render_pass_attachment_info(prosper::SampleCountFlags sampleCount)
{
	return prosper::util::RenderPassCreateInfo::AttachmentInfo {ShaderGameWorldLightingPass::RENDER_PASS_DEPTH_FORMAT, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Clear, prosper::AttachmentStoreOp::Store, sampleCount,
	  prosper::ImageLayout::DepthStencilAttachmentOptimal};
}

ShaderPrepassBase::ShaderPrepassBase(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader) : ShaderGameWorld(context, identifier, vsShader, fsShader)
{
	//SetPipelineCount(umath::to_integral(Pipeline::Count));
}
ShaderPrepassBase::ShaderPrepassBase(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorld(context, identifier, "programs/scene/prepass/prepass_depth", "")
{
	//SetPipelineCount(umath::to_integral(Pipeline::Count));
}

void ShaderPrepassBase::OnPipelinesInitialized() { ShaderGameWorld::OnPipelinesInitialized(); }

void ShaderPrepassBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { CreateCachedRenderPass<ShaderPrepassBase>({{get_depth_render_pass_attachment_info(GetSampleCount(pipelineIdx))}}, outRenderPass, pipelineIdx); }

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPrepassBase::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto *diffuseMap = mat.GetDiffuseMap();
	if(diffuseMap == nullptr || diffuseMap->texture == nullptr)
		return nullptr;
	auto diffuseTexture = std::static_pointer_cast<Texture>(diffuseMap->texture);
	if(diffuseTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this, descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*diffuseTexture->GetVkTexture(), umath::to_integral(MaterialBinding::AlbedoMap));
	descSet.Update();
	return descSetGroup;
}

void ShaderPrepassBase::InitializeShaderResources()
{
	ShaderEntity::InitializeShaderResources();

	AddVertexAttribute(VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(VERTEX_ATTRIBUTE_UV);

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit);

	AddDescriptorSetGroup(DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_RENDER_SETTINGS);
	m_materialDescSetInfo = std::make_unique<prosper::DescriptorSetInfo>(DESCRIPTOR_SET_MATERIAL);
}

void ShaderPrepassBase::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderEntity::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	pipelineInfo.ToggleDepthWrites(true);
	pipelineInfo.ToggleDepthTest(true, prosper::CompareOp::Less);
	ToggleDynamicScissorState(pipelineInfo, true);

	//pipelineInfo.ToggleDepthBias(true,0.f,0.f,0.f);
	//pipelineInfo.ToggleDynamicState(true,prosper::DynamicState::DepthBias); // Required for decals
}

uint32_t ShaderPrepassBase::GetCameraDescriptorSetIndex() const { return DESCRIPTOR_SET_SCENE.setIndex; }
uint32_t ShaderPrepassBase::GetInstanceDescriptorSetIndex() const { return DESCRIPTOR_SET_INSTANCE.setIndex; }
uint32_t ShaderPrepassBase::GetRenderSettingsDescriptorSetIndex() const { return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex; }
void ShaderPrepassBase::GetVertexAnimationPushConstantInfo(uint32_t &offset) const { offset = offsetof(PushConstants, vertexAnimInfo); }

//

void ShaderPrepassBase::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	std::array<prosper::IDescriptorSet *, 2> descSets {&dsScene, &dsRenderSettings};

	ShaderPrepass::PushConstants pushConstants {};
	pushConstants.Initialize();
	pushConstants.drawOrigin = drawOrigin;
	pushConstants.flags = inOutSceneFlags;
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, 0u, sizeof(pushConstants), &pushConstants);

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetSceneDescriptorSetIndex(), descSets, dynamicOffsets);
}

uint32_t ShaderPrepassBase::GetSceneDescriptorSetIndex() const { return DESCRIPTOR_SET_SCENE.setIndex; }

void ShaderPrepassBase::RecordAlphaCutoff(rendering::ShaderProcessor &shaderProcessor, float alphaCutoff) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(PushConstants, alphaCutoff), sizeof(alphaCutoff), &alphaCutoff);
}

bool ShaderPrepassBase::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const
{
	if(mat.GetAlphaMode() == AlphaMode::Opaque)
		return false;
	return ShaderGameWorld::RecordBindMaterial(shaderProcessor, mat);
}

//////////////////

decltype(ShaderPrepass::VERTEX_ATTRIBUTE_NORMAL) ShaderPrepass::VERTEX_ATTRIBUTE_NORMAL = {ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_NORMAL, VERTEX_BINDING_VERTEX};

decltype(ShaderPrepass::RENDER_PASS_NORMAL_FORMAT) ShaderPrepass::RENDER_PASS_NORMAL_FORMAT = prosper::Format::R16G16B16A16_SFloat;

prosper::util::RenderPassCreateInfo::AttachmentInfo ShaderPrepass::get_normal_render_pass_attachment_info(prosper::SampleCountFlags sampleCount)
{
	return prosper::util::RenderPassCreateInfo::AttachmentInfo {RENDER_PASS_NORMAL_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal};
}

ShaderPrepass::ShaderPrepass(prosper::IPrContext &context, const std::string &identifier) : ShaderPrepassBase(context, identifier, "programs/scene/prepass/prepass", "programs/scene/prepass/prepass")
{
	// SetBaseShader<ShaderTextured3DBase>();
	SetPipelineCount(umath::to_integral(Pipeline::Count) * umath::to_integral(rendering::PassType::Count));
}

uint32_t ShaderPrepass::GetPassPipelineIndexStartOffset(rendering::PassType passType) const { return umath::to_integral(passType) * umath::to_integral(Pipeline::Count); }

void ShaderPrepass::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	prosper::util::RenderPassCreateInfo rpInfo {{get_normal_render_pass_attachment_info(sampleCount), get_depth_render_pass_attachment_info(sampleCount)}};
	CreateCachedRenderPass<ShaderPrepass>({rpInfo}, outRenderPass, pipelineIdx);
}

void ShaderPrepass::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderPrepassBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	auto enableAlphaTest = false;
	auto enableNormalOutput = false;
	auto enableAnimation = false;
	auto enableMorphTargetAnimation = false;
	auto extendedVertexWeights = false;
	switch(static_cast<Pipeline>(pipelineIdx % umath::to_integral(Pipeline::Count))) {
	case Pipeline::Opaque:
		enableAnimation = true;
		enableMorphTargetAnimation = true;
		break;
	case Pipeline::AlphaTest:
		enableAlphaTest = true;

		enableAnimation = true;
		enableMorphTargetAnimation = true;
		break;
	case Pipeline::AnimatedOpaque:
		enableAnimation = true;
		enableMorphTargetAnimation = true;
		break;
	case Pipeline::AnimatedAlphaTest:
		enableAlphaTest = true;
		enableAnimation = true;
		enableMorphTargetAnimation = true;
		break;
	}

	auto isReflection = (static_cast<rendering::PassType>(pipelineIdx / umath::to_integral(Pipeline::Count)) == rendering::PassType::Reflection);
	if(isReflection)
		prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo, prosper::CullModeFlags::FrontBit);

	if(client->GetGameWorldShaderSettings().ssaoEnabled)
		enableNormalOutput = true;
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, umath::to_integral(SpecializationConstant::EnableAlphaTest), static_cast<uint32_t>(enableAlphaTest));
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, umath::to_integral(SpecializationConstant::EnableNormalOutput), static_cast<uint32_t>(enableNormalOutput));
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::VertexBit, umath::to_integral(SpecializationConstant::EnableAnimation), static_cast<uint32_t>(enableAnimation));
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::VertexBit, umath::to_integral(SpecializationConstant::EnableMorphTargetAnimation), static_cast<uint32_t>(enableMorphTargetAnimation));
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::VertexBit, umath::to_integral(SpecializationConstant::EnableExtendedVertexWeights), static_cast<uint32_t>(extendedVertexWeights));
}
void ShaderPrepass::InitializeShaderResources()
{
	ShaderPrepassBase::InitializeShaderResources();
	AddVertexAttribute(VERTEX_ATTRIBUTE_NORMAL);
}
