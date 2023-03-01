/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_skybox.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
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
decltype(ShaderSkybox::DESCRIPTOR_SET_MATERIAL) ShaderSkybox::DESCRIPTOR_SET_MATERIAL = {{prosper::DescriptorSetInfo::Binding {// Skybox Map
  prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}}};
ShaderSkybox::ShaderSkybox(prosper::IPrContext &context, const std::string &identifier) : ShaderSkybox(context, identifier, "world/vs_skybox", "world/fs_skybox") {}

ShaderSkybox::ShaderSkybox(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader) : ShaderGameWorldLightingPass(context, identifier, vsShader, fsShader)
{
	// SetBaseShader<ShaderTextured3DBase>();
}

prosper::DescriptorSetInfo &ShaderSkybox::GetMaterialDescriptorSetInfo() const { return DESCRIPTOR_SET_MATERIAL; }

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

	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_POSITION);

	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_MATERIAL);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_RENDERER);
	ToggleDynamicScissorState(pipelineInfo, true);
	InitializeGfxPipelinePushConstantRanges(pipelineInfo, pipelineIdx);
}

uint32_t ShaderSkybox::GetRenderSettingsDescriptorSetIndex() const { return std::numeric_limits<uint32_t>::max(); }
uint32_t ShaderSkybox::GetRendererDescriptorSetIndex() const { return DESCRIPTOR_SET_RENDERER.setIndex; }
uint32_t ShaderSkybox::GetCameraDescriptorSetIndex() const { return std::numeric_limits<uint32_t>::max(); }
uint32_t ShaderSkybox::GetLightDescriptorSetIndex() const { return std::numeric_limits<uint32_t>::max(); }
uint32_t ShaderSkybox::GetInstanceDescriptorSetIndex() const { return DESCRIPTOR_SET_INSTANCE.setIndex; }
uint32_t ShaderSkybox::GetMaterialDescriptorSetIndex() const { return DESCRIPTOR_SET_MATERIAL.setIndex; }

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderSkybox::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto *skyboxMap = mat.GetTextureInfo("skybox");
	if(skyboxMap == nullptr || skyboxMap->texture == nullptr)
		return nullptr;
	auto skyboxTexture = std::static_pointer_cast<Texture>(skyboxMap->texture);
	if(skyboxTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this, descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*skyboxTexture->GetVkTexture(), 0u);
	descSet.Update();
	return descSetGroup;
}
//

void ShaderSkybox::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, prosper::IDescriptorSet &dsMaterial, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	std::array<prosper::IDescriptorSet *, 3> descSets {&dsMaterial, &dsScene, &dsRenderer};

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), pragma::ShaderGameWorld::MATERIAL_DESCRIPTOR_SET_INDEX, descSets, dynamicOffsets);
}

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
	return cmd.RecordPushConstants(layout, prosper::ShaderStageFlags::VertexBit, sizeof(ShaderGameWorldLightingPass::PushConstants), sizeof(PushConstants), &pushConstants);
}

void ShaderSkybox::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}

//////////////

ShaderSkyboxEquirect::ShaderSkyboxEquirect(prosper::IPrContext &context, const std::string &identifier) : ShaderSkybox {context, identifier, "world/vs_skybox_equirect", "world/fs_skybox_equirect"} {}
