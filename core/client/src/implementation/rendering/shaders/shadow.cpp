// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cstddef>

module pragma.client;

import :rendering.shaders.shadow;

import :engine;
import :entities.components;

using namespace pragma;

static auto SHADOW_DEPTH_BIAS_CONSTANT = 1.25f;
static auto SHADOW_DEPTH_BIAS_SLOPE = 1.75f;

decltype(ShaderShadow::RENDER_PASS_DEPTH_FORMAT) ShaderShadow::RENDER_PASS_DEPTH_FORMAT = prosper::Format::D32_SFloat;

decltype(ShaderShadow::VERTEX_BINDING_RENDER_BUFFER_INDEX) ShaderShadow::VERTEX_BINDING_RENDER_BUFFER_INDEX = {prosper::VertexInputRate::Instance};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX) ShaderShadow::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX = {ShaderGameWorld::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX, VERTEX_BINDING_RENDER_BUFFER_INDEX};

decltype(ShaderShadow::VERTEX_BINDING_BONE_WEIGHT) ShaderShadow::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderGameWorld::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID, VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderGameWorld::VERTEX_ATTRIBUTE_BONE_WEIGHT, VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderShadow::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderShadow::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {ShaderGameWorld::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID, VERTEX_BINDING_BONE_WEIGHT_EXT};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {ShaderGameWorld::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT, VERTEX_BINDING_BONE_WEIGHT_EXT};

decltype(ShaderShadow::VERTEX_BINDING_VERTEX) ShaderShadow::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex, sizeof(rendering::VertexBufferData)};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_POSITION) ShaderShadow::VERTEX_ATTRIBUTE_POSITION = {ShaderGameWorld::VERTEX_ATTRIBUTE_POSITION, VERTEX_BINDING_VERTEX};

decltype(ShaderShadow::DESCRIPTOR_SET_INSTANCE) ShaderShadow::DESCRIPTOR_SET_INSTANCE = {&ShaderGameWorld::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderShadow::DESCRIPTOR_SET_SCENE) ShaderShadow::DESCRIPTOR_SET_SCENE = {&ShaderScene::DESCRIPTOR_SET_SCENE};
decltype(ShaderShadow::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderShadow::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderGameWorld::DESCRIPTOR_SET_RENDER_SETTINGS};
ShaderShadow::ShaderShadow(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader) : ShaderGameWorld(context, identifier, vsShader, fsShader) { SetPipelineCount(math::to_integral(Pipeline::Count)); }

ShaderShadow::ShaderShadow(prosper::IPrContext &context, const std::string &identifier) : ShaderShadow(context, identifier, "programs/lighting/shadow/shadow", "programs/lighting/shadow/shadow") {}
void ShaderShadow::OnPipelinesInitialized() { ShaderGameWorld::OnPipelinesInitialized(); }

void ShaderShadow::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderShadow>({{{{RENDER_PASS_DEPTH_FORMAT, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Clear, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::ShaderReadOnlyOptimal}}}}, outRenderPass,
	  pipelineIdx);
}
uint32_t ShaderShadow::GetRenderSettingsDescriptorSetIndex() const { return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex; }
uint32_t ShaderShadow::GetCameraDescriptorSetIndex() const { return std::numeric_limits<uint32_t>::max(); }
uint32_t ShaderShadow::GetInstanceDescriptorSetIndex() const { return DESCRIPTOR_SET_INSTANCE.setIndex; }
void ShaderShadow::GetVertexAnimationPushConstantInfo(uint32_t &offset) const { offset = offsetof(PushConstants, vertexAnimInfo); }
void ShaderShadow::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddVertexAttribute(VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit);

	AddDescriptorSetGroup(DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_RENDER_SETTINGS);
}
void ShaderShadow::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo, true);
	pipelineInfo.ToggleDepthWrites(true);
	pipelineInfo.ToggleDepthTest(true, prosper::CompareOp::LessOrEqual);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo, prosper::CullModeFlags::None);

	pipelineInfo.ToggleDepthBias(true, SHADOW_DEPTH_BIAS_CONSTANT, 0.f, SHADOW_DEPTH_BIAS_SLOPE);
	uint32_t enableMorphTagetAnimations = (pipelineIdx == math::to_integral(Pipeline::WithMorphTargetAnimations));
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::VertexBit, math::get_least_significant_set_bit_index(math::to_integral(GameShaderSpecializationConstantFlag::EnableMorphTargetAnimationBit)), sizeof(enableMorphTagetAnimations), &enableMorphTagetAnimations);
}

uint32_t ShaderShadow::GetSceneDescriptorSetIndex() const { return DESCRIPTOR_SET_SCENE.setIndex; }

void ShaderShadow::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const
{
	std::array<prosper::IDescriptorSet *, 2> descSets {&dsScene, &dsRenderSettings};

	PushConstants pushConstants {};
	pushConstants.Initialize();
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, 0u, sizeof(pushConstants), &pushConstants);

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetSceneDescriptorSetIndex(), descSets, dynamicOffsets);
}

void ShaderShadow::RecordSceneFlags(rendering::ShaderProcessor &shaderProcessor, SceneFlags sceneFlags) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(PushConstants, flags), sizeof(sceneFlags), &sceneFlags);
}

void ShaderShadow::RecordBindLight(rendering::ShaderProcessor &shaderProcessor, CLightComponent &light, uint32_t layerId) const
{
#pragma pack(push, 1)
	struct {
		Mat4 depthMVP;
		Vector4 lightPos;
	} pushData;
#pragma pack(pop)

	auto pRadiusComponent = light.GetEntity().GetComponent<CRadiusComponent>();
	auto &pos = light.GetEntity().GetPosition();
	pushData.depthMVP = light.GetTransformationMatrix(layerId);
	pushData.lightPos = Vector4 {pos.x, pos.y, pos.z, static_cast<float>(pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f)};

	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(PushConstants, depthMVP), sizeof(pushData), &pushData);
}

void ShaderShadow::RecordAlphaCutoff(rendering::ShaderProcessor &shaderProcessor, float alphaCutoff) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(PushConstants, alphaCutoff), sizeof(alphaCutoff), &alphaCutoff);
}

bool ShaderShadow::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, material::CMaterial &mat) const
{
	if(mat.GetAlphaMode() == AlphaMode::Opaque)
		return false;
	return ShaderGameWorld::RecordBindMaterial(shaderProcessor, mat);
}

void ShaderShadow::RecordVertexAnimationOffset(rendering::ShaderProcessor &shaderProcessor, uint32_t vertexAnimationOffset) const
{
	// TODO: Only if pipeline 1
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(PushConstants, vertexAnimInfo), sizeof(vertexAnimationOffset), &vertexAnimationOffset);
}

//////////////////

ShaderShadowSpot::ShaderShadowSpot(prosper::IPrContext &context, const std::string &identifier) : ShaderShadow(context, identifier, "programs/lighting/shadow/shadow", "programs/lighting/shadow/shadow_spot") { SetBaseShader<ShaderShadow>(); }

//////////////////

ShaderShadowCSM::ShaderShadowCSM(prosper::IPrContext &context, const std::string &identifier) : ShaderShadow(context, identifier, "programs/lighting/shadow/shadow_csm", "") { SetBaseShader<ShaderShadow>(); }
void ShaderShadowCSM::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderShadowCSM>({{{{RENDER_PASS_DEPTH_FORMAT, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::ShaderReadOnlyOptimal}}}},
	  outRenderPass, pipelineIdx);
}
