// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_skybox;

import :engine;
import :entities.components;
import :model;

using namespace pragma;

decltype(ShaderSkybox::VERTEX_BINDING_VERTEX) ShaderSkybox::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex, sizeof(rendering::VertexBufferData)};
decltype(ShaderSkybox::VERTEX_ATTRIBUTE_POSITION) ShaderSkybox::VERTEX_ATTRIBUTE_POSITION = {ShaderGameWorldLightingPass::VERTEX_ATTRIBUTE_POSITION, VERTEX_BINDING_VERTEX};
decltype(ShaderSkybox::DESCRIPTOR_SET_INSTANCE) ShaderSkybox::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderSkybox::DESCRIPTOR_SET_SCENE) ShaderSkybox::DESCRIPTOR_SET_SCENE = {&ShaderEntity::DESCRIPTOR_SET_SCENE};
decltype(ShaderSkybox::DESCRIPTOR_SET_RENDERER) ShaderSkybox::DESCRIPTOR_SET_RENDERER = {&ShaderEntity::DESCRIPTOR_SET_RENDERER};
ShaderSkybox::ShaderSkybox(prosper::IPrContext &context, const std::string &identifier) : ShaderSkybox(context, identifier, "programs/scene/skybox/skybox", "programs/scene/skybox/skybox") {}

ShaderSkybox::ShaderSkybox(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader) : ShaderGameWorldLightingPass(context, identifier, vsShader, fsShader)
{
	// SetBaseShader<ShaderTextured3DBase>();
	m_shaderMaterialName = "skybox";
}

bool ShaderSkybox::GetRenderBufferTargets(geometry::CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const
{
	auto &sceneMesh = mesh.GetSceneMesh();
	if(sceneMesh == nullptr)
		return false;
	auto &indexBuffer = sceneMesh->GetIndexBuffer();
	if(indexBuffer) {
		outIndexBufferInfo = prosper::IndexBufferInfo {};
		outIndexBufferInfo->buffer = indexBuffer;
		outIndexBufferInfo->indexType = (mesh.GetIndexType() == geometry::IndexType::UInt16) ? prosper::IndexType::UInt16 : prosper::IndexType::UInt32;
		outIndexBufferInfo->offset = 0;
	}

	auto *vBuf = sceneMesh->GetVertexBuffer().get();
	outBuffers = {vBuf};
	outOffsets = {0ull};
	return true;
}

void ShaderSkybox::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderEntity::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	auto isReflection = (static_cast<rendering::PassType>(GetBasePassType(pipelineIdx)) == rendering::PassType::Reflection);
	if(isReflection) {
		prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo, prosper::CullModeFlags::FrontBit);
		//pipelineInfo.SetRasterizationProperties(prosper::PolygonMode::Line,prosper::CullModeFlags::FrontBit,prosper::FrontFace::Clockwise,1.f);
	}

	pipelineInfo.ToggleDepthWrites(false);
	pipelineInfo.ToggleDepthTest(false, prosper::CompareOp::Always);

	ToggleDynamicScissorState(pipelineInfo, true);
}
void ShaderSkybox::InitializeShaderResources()
{
	ShaderEntity::InitializeShaderResources();
	InitializeShaderMaterial();

	AddVertexAttribute(VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);
	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);

	AddDescriptorSetGroup(DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_RENDERER);
	AddDescriptorSetGroup(GetMaterialDescriptorSetInfo());
	InitializeGfxPipelinePushConstantRanges();
}

uint32_t ShaderSkybox::GetRenderSettingsDescriptorSetIndex() const { return std::numeric_limits<uint32_t>::max(); }
uint32_t ShaderSkybox::GetRendererDescriptorSetIndex() const { return DESCRIPTOR_SET_RENDERER.setIndex; }
uint32_t ShaderSkybox::GetCameraDescriptorSetIndex() const { return std::numeric_limits<uint32_t>::max(); }
uint32_t ShaderSkybox::GetInstanceDescriptorSetIndex() const { return DESCRIPTOR_SET_INSTANCE.setIndex; }

void ShaderSkybox::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const
{
	std::array<prosper::IDescriptorSet *, 2> descSets {&dsScene, &dsRenderer};

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetSceneDescriptorSetIndex(), descSets, dynamicOffsets);
}

uint32_t ShaderSkybox::GetSceneDescriptorSetIndex() const { return DESCRIPTOR_SET_SCENE.setIndex; }

bool ShaderSkybox::RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const
{
	if(ShaderGameWorldLightingPass::RecordBindEntity(shaderProcessor, renderC, layout, entityInstanceDescriptorSetIndex) == false)
		return false;
	auto skyC = renderC.GetEntity().GetComponent<CSkyboxComponent>();
	if(skyC.expired())
		return false;
	auto &cmd = shaderProcessor.GetCommandBuffer();
	auto &pushConstants = skyC->GetRenderSkyAngles();
	static_assert(sizeof(PushConstants) == sizeof(pushConstants));
	return cmd.RecordPushConstants(layout, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit, sizeof(ShaderGameWorldLightingPass::PushConstants), sizeof(PushConstants), &pushConstants);
}

void ShaderSkybox::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }

//////////////

ShaderSkyboxEquirect::ShaderSkyboxEquirect(prosper::IPrContext &context, const std::string &identifier) : ShaderSkybox {context, identifier, "world/vs_skybox_equirect", "world/fs_skybox_equirect"} {}
