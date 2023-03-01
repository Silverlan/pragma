/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/lighting/c_light_data_buffer_manager.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <cmaterial.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

decltype(ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS = {{prosper::DescriptorSetInfo::Binding {// Debug
                                                                                                        prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Time
    prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit},
  prosper::DescriptorSetInfo::Binding {// CSM Data
    prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Global Entity Instance Data
    prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit}}};
decltype(ShaderScene::DESCRIPTOR_SET_SCENE) ShaderScene::DESCRIPTOR_SET_SCENE = {{prosper::DescriptorSetInfo::Binding {// Camera
                                                                                    prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit},
  prosper::DescriptorSetInfo::Binding {// Render Settings
    prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit}}};
decltype(ShaderScene::DESCRIPTOR_SET_RENDERER) ShaderScene::DESCRIPTOR_SET_RENDERER = {{prosper::DescriptorSetInfo::Binding {// Renderer
                                                                                          prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// SSAO Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Light Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Indirect Light Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Directional Light Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}}};
decltype(ShaderScene::RENDER_PASS_FORMAT) ShaderScene::RENDER_PASS_FORMAT = prosper::Format::R16G16B16A16_SFloat;
decltype(ShaderScene::RENDER_PASS_DEPTH_FORMAT) ShaderScene::RENDER_PASS_DEPTH_FORMAT = prosper::Format::D32_SFloat;
decltype(ShaderScene::RENDER_PASS_SAMPLES) ShaderScene::RENDER_PASS_SAMPLES = prosper::SampleCountFlags::e1Bit;
void ShaderScene::SetRenderPassSampleCount(prosper::SampleCountFlags samples) { RENDER_PASS_SAMPLES = samples; }
ShaderScene::ShaderScene(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : Shader3DBase(context, identifier, vsShader, fsShader, gsShader)
{
	//SetPipelineCount(umath::to_integral(Pipeline::Count));
}
bool ShaderScene::ShouldInitializePipeline(uint32_t pipelineIdx)
{
	//if(static_cast<Pipeline>(pipelineIdx) != Pipeline::MultiSample)
	//	return true;
	//return RENDER_PASS_SAMPLES != prosper::SampleCountFlags::e1Bit;
	return true;
}
prosper::SampleCountFlags ShaderScene::GetSampleCount(uint32_t pipelineIdx) const
{
	return prosper::SampleCountFlags::e1Bit; //(static_cast<Pipeline>(pipelineIdx) == Pipeline::MultiSample) ? RENDER_PASS_SAMPLES : prosper::SampleCountFlags::e1Bit;
}
void ShaderScene::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	prosper::util::RenderPassCreateInfo rpCreateInfo {{{RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal},
	  {// Bloom Attachment
	    RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Clear, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal},
	  {RENDER_PASS_DEPTH_FORMAT, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store /* depth values have already been written by prepass */, sampleCount, prosper::ImageLayout::DepthStencilAttachmentOptimal}}};
	rpCreateInfo.subPasses.push_back(prosper::util::RenderPassCreateInfo::SubPass {std::vector<std::size_t> {0ull, 1ull}, true});
	/*auto mainSubPassId = rpCreateInfo.subPasses.size() -1ull;

	// Particle sub-pass
	rpCreateInfo.subPasses.push_back(prosper::util::RenderPassCreateInfo::SubPass{std::vector<std::size_t>{0ull,1ull},true});
	auto particleSubPassId = rpCreateInfo.subPasses.size() -1ull;
	auto &subPass = rpCreateInfo.subPasses.back();
	subPass.dependencies.push_back({
		particleSubPassId,mainSubPassId,
		vk::PipelineStageFlagBits::eFragmentShader,vk::PipelineStageFlagBits::eFragmentShader,
		prosper::AccessFlags::ShaderWriteBit | prosper::AccessFlags::ShaderReadBit,prosper::AccessFlags::ShaderWriteBit
	});*/
	CreateCachedRenderPass<ShaderScene>(rpCreateInfo, outRenderPass, pipelineIdx);
}

/////////////////////

decltype(ShaderSceneLit::DESCRIPTOR_SET_LIGHTS) ShaderSceneLit::DESCRIPTOR_SET_LIGHTS = {{prosper::DescriptorSetInfo::Binding {// Light sources
                                                                                            LIGHT_SOURCE_BUFFER_TYPE, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Visible light index buffer
    prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit},
  prosper::DescriptorSetInfo::Binding {// Shadow buffers
    LIGHT_SOURCE_BUFFER_TYPE, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Cascade Maps
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, umath::to_integral(GameLimits::MaxCSMCascades)}}};
decltype(ShaderSceneLit::DESCRIPTOR_SET_SHADOWS) ShaderSceneLit::DESCRIPTOR_SET_SHADOWS = {{prosper::DescriptorSetInfo::Binding {// Shadow Maps
                                                                                              prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, umath::to_integral(GameLimits::MaxActiveShadowMaps)},
  prosper::DescriptorSetInfo::Binding {// Shadow Cube-Maps
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, umath::to_integral(GameLimits::MaxActiveShadowCubeMaps)}}};
ShaderSceneLit::ShaderSceneLit(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderScene(context, identifier, vsShader, fsShader, gsShader) {}

/////////////////////

decltype(ShaderGameWorld::HASH_TYPE) ShaderGameWorld::HASH_TYPE = typeid(ShaderGameWorld).hash_code();
size_t ShaderGameWorld::GetBaseTypeHashCode() const { return HASH_TYPE; }
prosper::IDescriptorSet &ShaderGameWorld::GetDefaultMaterialDescriptorSet() const { return *m_defaultMatDsg->GetDescriptorSet(); }

/////////////////////

decltype(ShaderEntity::VERTEX_BINDING_RENDER_BUFFER_INDEX) ShaderEntity::VERTEX_BINDING_RENDER_BUFFER_INDEX = {prosper::VertexInputRate::Instance};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX) ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX = {VERTEX_BINDING_RENDER_BUFFER_INDEX, prosper::Format::R32_UInt};

decltype(ShaderEntity::VERTEX_BINDING_BONE_WEIGHT) ShaderEntity::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {VERTEX_BINDING_BONE_WEIGHT, prosper::Format::R32G32B32A32_SInt};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT = {VERTEX_BINDING_BONE_WEIGHT, prosper::Format::R32G32B32A32_SFloat};

decltype(ShaderEntity::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderEntity::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {VERTEX_BINDING_BONE_WEIGHT_EXT, prosper::Format::R32G32B32A32_SInt};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {VERTEX_BINDING_BONE_WEIGHT_EXT, prosper::Format::R32G32B32A32_SFloat};

decltype(ShaderEntity::VERTEX_BINDING_VERTEX) ShaderEntity::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex, sizeof(VertexBufferData)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_POSITION) ShaderEntity::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_UV) ShaderEntity::VERTEX_ATTRIBUTE_UV = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32_SFloat, offsetof(VertexBufferData, uv)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_NORMAL) ShaderEntity::VERTEX_ATTRIBUTE_NORMAL = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat, offsetof(VertexBufferData, normal)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_TANGENT) ShaderEntity::VERTEX_ATTRIBUTE_TANGENT = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat, offsetof(VertexBufferData, tangent)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT) ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat, offsetof(VertexBufferData, biTangent)};

decltype(ShaderEntity::VERTEX_BINDING_LIGHTMAP) ShaderEntity::VERTEX_BINDING_LIGHTMAP = {prosper::VertexInputRate::Vertex};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV) ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV = {VERTEX_BINDING_LIGHTMAP, prosper::Format::R32G32_SFloat};

decltype(ShaderEntity::DESCRIPTOR_SET_INSTANCE) ShaderEntity::DESCRIPTOR_SET_INSTANCE = {{prosper::DescriptorSetInfo::Binding {// Instance
                                                                                            prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit},
  prosper::DescriptorSetInfo::Binding {// Bone Matrices
    prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::VertexBit},
  prosper::DescriptorSetInfo::Binding {// Vertex Animations
    prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::VertexBit},
  prosper::DescriptorSetInfo::Binding {// Vertex Animation Frame Data
    prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::VertexBit}}};
ShaderEntity::ShaderEntity(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderSceneLit(context, identifier, vsShader, fsShader, gsShader) {}

bool ShaderEntity::GetRenderBufferTargets(CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const
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

	auto *vweightBuf = sceneMesh->GetVertexWeightBuffer().get();
	auto *vweightExtBuf = sceneMesh->GetVertexWeightBuffer().get();
	auto *vBuf = sceneMesh->GetVertexBuffer().get();

	outBuffers = {vweightBuf, vweightBuf, vBuf};
	auto &vertWeights = mesh.GetVertexWeights();
	outOffsets = {0ull, vertWeights.size() * sizeof(vertWeights.front()), 0ull};
	return true;
}
std::shared_ptr<prosper::IRenderBuffer> ShaderEntity::CreateRenderBuffer(CModelSubMesh &mesh, uint32_t pipelineIdx) const
{
	std::vector<prosper::IBuffer *> buffers;
	std::vector<prosper::DeviceSize> offsets;
	std::optional<prosper::IndexBufferInfo> indexBufferInfo {};
	if(GetRenderBufferTargets(mesh, pipelineIdx, buffers, offsets, indexBufferInfo) == false)
		return nullptr;
	buffers.insert(buffers.begin(), CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer().get()); // Instance buffer
	offsets.insert(offsets.begin(), 0);
	auto *dummyBuffer = c_engine->GetRenderContext().GetDummyBuffer().get();
	for(auto it = buffers.begin(); it != buffers.end(); ++it) {
		auto *buf = *it;
		*it = buf ? buf : dummyBuffer;
	}
	return GetContext().CreateRenderBuffer(static_cast<const prosper::GraphicsPipelineCreateInfo &>(*GetPipelineCreateInfo(pipelineIdx)), buffers, offsets, indexBufferInfo);
}

/////////////

void pragma::ShaderGameWorld::RecordSceneFlags(rendering::ShaderProcessor &shaderProcessor, SceneFlags sceneFlags) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(ScenePushConstants, flags), sizeof(sceneFlags), &sceneFlags);
}

bool pragma::ShaderGameWorld::RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const
{
	return shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, layout, entityInstanceDescriptorSetIndex, *renderC.GetRenderDescriptorSet());
}

bool pragma::ShaderGameWorld::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const
{
	auto descSetGroup = mat.GetDescriptorSetGroup(const_cast<pragma::ShaderGameWorld &>(*this));
	// if(descSetGroup == nullptr)
	// 	descSetGroup = const_cast<pragma::ShaderGameWorld*>(this)->InitializeMaterialDescriptorSet(mat,false); // Attempt to initialize on the fly (TODO: Is this thread safe?)
	if(descSetGroup == nullptr)
		return false;
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetMaterialDescriptorSetIndex(), *descSetGroup->GetDescriptorSet(0));
	return true;
}

void pragma::ShaderGameWorld::RecordClipPlane(rendering::ShaderProcessor &shaderProcessor, const Vector4 &clipPlane) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(pragma::ShaderGameWorld::ScenePushConstants, clipPlane), sizeof(clipPlane), &clipPlane);
}

void pragma::ShaderGameWorld::RecordDrawOrigin(rendering::ShaderProcessor &shaderProcessor, const Vector4 &drawOrigin) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(pragma::ShaderGameWorld::ScenePushConstants, drawOrigin), sizeof(drawOrigin), &drawOrigin);
}

void pragma::ShaderGameWorld::RecordDepthBias(rendering::ShaderProcessor &shaderProcessor, const Vector2 &depthBias) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(pragma::ShaderGameWorld::ScenePushConstants, depthBias), sizeof(depthBias), &depthBias);
}

void pragma::ShaderGameWorld::RecordVertexAnimationOffset(rendering::ShaderProcessor &shaderProcessor, uint32_t vertexAnimationOffset) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(pragma::ShaderGameWorld::ScenePushConstants, vertexAnimInfo), sizeof(vertexAnimationOffset),
	  &vertexAnimationOffset);
}
