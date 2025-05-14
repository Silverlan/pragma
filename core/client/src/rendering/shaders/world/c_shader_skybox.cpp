/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_skybox.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/environment/c_sky_camera.hpp"
#include "pragma/entities/c_skybox.h"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderSkybox::VERTEX_BINDING_VERTEX) ShaderSkybox::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex, sizeof(VertexBufferData)};
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

bool ShaderSkybox::GetRenderBufferTargets(CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const
{
	auto &sceneMesh = mesh.GetSceneMesh();
	if(sceneMesh == nullptr)
		return false;
	auto &indexBuffer = sceneMesh->GetIndexBuffer();
	if(indexBuffer) {
		outIndexBufferInfo = prosper::IndexBufferInfo {};
		outIndexBufferInfo->buffer = indexBuffer;
		outIndexBufferInfo->indexType = (mesh.GetIndexType() == pragma::model::IndexType::UInt16) ? prosper::IndexType::UInt16 : prosper::IndexType::UInt32;
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

void ShaderSkybox::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
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
