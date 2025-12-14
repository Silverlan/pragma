// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cstddef>

module pragma.client;

import :rendering.shaders.scene;

import :client_state;
import :engine;
import :entities.components;
import :model;
import :rendering.light_data_buffer_manager;

using namespace pragma;

decltype(ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS = {"RENDER_SETTINGS",
  {
    prosper::DescriptorSetInfo::Binding {"DEBUG", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"TIME", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit},
    prosper::DescriptorSetInfo::Binding {"CSM_DATA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"GLOBAL_ENTITY_INSTANCE_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit},
#ifdef PRAGMA_ENABLE_SHADER_DEBUG_PRINT
    prosper::DescriptorSetInfo::Binding {"DEBUG_PRINT", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit},
#endif
  }};
decltype(ShaderScene::DESCRIPTOR_SET_SCENE) ShaderScene::DESCRIPTOR_SET_SCENE = {
  "SCENE",
  {prosper::DescriptorSetInfo::Binding {"CAMERA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit},
    prosper::DescriptorSetInfo::Binding {"RENDER_SETTINGS", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit}},
};
decltype(ShaderScene::DESCRIPTOR_SET_RENDERER) ShaderScene::DESCRIPTOR_SET_RENDERER = {
  "RENDERER",
  {prosper::DescriptorSetInfo::Binding {"RENDERER", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"SSAO_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"LIGHT_BUFFERS", LIGHT_SOURCE_BUFFER_TYPE, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"VISIBLE_LIGHT_TILE_INDEX_BUFFER", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit},
    prosper::DescriptorSetInfo::Binding {"SHADOW_BUFFERS", LIGHT_SOURCE_BUFFER_TYPE, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"CSM_MAPS", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, math::to_integral(GameLimits::MaxCSMCascades)},
    prosper::DescriptorSetInfo::Binding {"LIGHTMAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"INDIRECT_LIGHTMAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"DIRECTIONAL_LIGHTMAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderScene::RENDER_PASS_FORMAT) ShaderScene::RENDER_PASS_FORMAT = prosper::Format::R16G16B16A16_SFloat;
decltype(ShaderScene::RENDER_PASS_DEPTH_FORMAT) ShaderScene::RENDER_PASS_DEPTH_FORMAT = prosper::Format::D32_SFloat;
decltype(ShaderScene::RENDER_PASS_SAMPLES) ShaderScene::RENDER_PASS_SAMPLES = prosper::SampleCountFlags::e1Bit;
void ShaderScene::SetRenderPassSampleCount(prosper::SampleCountFlags samples) { RENDER_PASS_SAMPLES = samples; }
ShaderScene::ShaderScene(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : Shader3DBase(context, identifier, vsShader, fsShader, gsShader)
{
	//SetPipelineCount(pragma::math::to_integral(Pipeline::Count));
}
bool ShaderScene::IsDebugPrintEnabled() const
{
#ifdef PRAGMA_ENABLE_SHADER_DEBUG_PRINT
	return true;
#else
	return false;
#endif
}
std::optional<std::string> ShaderScene::GetGlslPrefixCode(prosper::ShaderStage stage) const
{
#ifdef PRAGMA_ENABLE_SHADER_DEBUG_PRINT
	if(IsDebugPrintEnabled() && (stage == prosper::ShaderStage::Fragment || stage == prosper::ShaderStage::Vertex))
		return "#include \"/debug/debug_print.glsl\"\n";
#endif
	return Shader3DBase::GetGlslPrefixCode(stage);
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
	prosper::util::RenderPassCreateInfo rpCreateInfo {
	  {{RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal},
	    {RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Clear, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal}, // Bloom Attachment
	    {RENDER_PASS_DEPTH_FORMAT, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store /* depth values have already been written by prepass */, sampleCount, prosper::ImageLayout::DepthStencilAttachmentOptimal}},
	};
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

decltype(ShaderSceneLit::DESCRIPTOR_SET_SHADOWS) ShaderSceneLit::DESCRIPTOR_SET_SHADOWS = {
  "SHADOWS",
  {prosper::DescriptorSetInfo::Binding {"MAPS", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, math::to_integral(GameLimits::MaxActiveShadowMaps)},
    prosper::DescriptorSetInfo::Binding {"CUBEMAPS", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, math::to_integral(GameLimits::MaxActiveShadowCubeMaps), std::numeric_limits<uint32_t>::max(),
      prosper::PrDescriptorSetBindingFlags::Cubemap}},
};
ShaderSceneLit::ShaderSceneLit(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderScene(context, identifier, vsShader, fsShader, gsShader) {}

/////////////////////

decltype(ShaderGameWorld::HASH_TYPE) ShaderGameWorld::HASH_TYPE = typeid(ShaderGameWorld).hash_code();
size_t ShaderGameWorld::GetBaseTypeHashCode() const { return HASH_TYPE; }
std::optional<uint32_t> ShaderGameWorld::GetMaterialDescriptorSetIndex() const { return m_materialDescSetInfo ? m_materialDescSetInfo->setIndex : std::optional<uint32_t> {}; }
const prosper::DescriptorSetInfo *ShaderGameWorld::GetMaterialDescriptorSetInfo() const { return m_materialDescSetInfo.get(); }

/////////////////////

decltype(ShaderEntity::VERTEX_BINDING_RENDER_BUFFER_INDEX) ShaderEntity::VERTEX_BINDING_RENDER_BUFFER_INDEX = {prosper::VertexInputRate::Instance};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX) ShaderEntity::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX = {VERTEX_BINDING_RENDER_BUFFER_INDEX, prosper::Format::R32_UInt};

decltype(ShaderEntity::VERTEX_BINDING_BONE_WEIGHT) ShaderEntity::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {VERTEX_BINDING_BONE_WEIGHT, prosper::Format::R32G32B32A32_SInt};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT = {VERTEX_BINDING_BONE_WEIGHT, prosper::Format::R32G32B32A32_SFloat};

decltype(ShaderEntity::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderEntity::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {VERTEX_BINDING_BONE_WEIGHT_EXT, prosper::Format::R32G32B32A32_SInt};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {VERTEX_BINDING_BONE_WEIGHT_EXT, prosper::Format::R32G32B32A32_SFloat};

decltype(ShaderEntity::VERTEX_BINDING_VERTEX) ShaderEntity::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex, sizeof(rendering::VertexBufferData)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_POSITION) ShaderEntity::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_UV) ShaderEntity::VERTEX_ATTRIBUTE_UV = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32_SFloat, offsetof(pragma::rendering::VertexBufferData, uv)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_NORMAL) ShaderEntity::VERTEX_ATTRIBUTE_NORMAL = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat, offsetof(pragma::rendering::VertexBufferData, normal)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_TANGENT) ShaderEntity::VERTEX_ATTRIBUTE_TANGENT = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat, offsetof(pragma::rendering::VertexBufferData, tangent)};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT) ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat, offsetof(pragma::rendering::VertexBufferData, biTangent)};

decltype(ShaderEntity::VERTEX_BINDING_LIGHTMAP) ShaderEntity::VERTEX_BINDING_LIGHTMAP = {prosper::VertexInputRate::Vertex};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV) ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV = {VERTEX_BINDING_LIGHTMAP, prosper::Format::R32G32_SFloat};

decltype(ShaderEntity::DESCRIPTOR_SET_INSTANCE) ShaderEntity::DESCRIPTOR_SET_INSTANCE = {
  "INSTANCE",
  {prosper::DescriptorSetInfo::Binding {"ENTITY_DATA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit},
    prosper::DescriptorSetInfo::Binding {"BONE_MATRICES", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::VertexBit}, prosper::DescriptorSetInfo::Binding {"VERTEX_ANIMATIONS", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::VertexBit},
    prosper::DescriptorSetInfo::Binding {"VERTEX_ANIMATION_FRAME_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::VertexBit}},
};
ShaderEntity::ShaderEntity(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderSceneLit(context, identifier, vsShader, fsShader, gsShader) {}

bool ShaderEntity::GetRenderBufferTargets(geometry::CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const
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

	auto *vweightBuf = sceneMesh->GetVertexWeightBuffer().get();
	auto *vweightExtBuf = sceneMesh->GetVertexWeightBuffer().get();
	auto *vBuf = sceneMesh->GetVertexBuffer().get();

	outBuffers = {vweightBuf, vweightBuf, vBuf};
	auto &vertWeights = mesh.GetVertexWeights();
	outOffsets = {0ull, vertWeights.size() * sizeof(vertWeights.front()), 0ull};
	return true;
}
std::shared_ptr<prosper::IRenderBuffer> ShaderEntity::CreateRenderBuffer(geometry::CModelSubMesh &mesh, uint32_t pipelineIdx) const
{
	std::vector<prosper::IBuffer *> buffers;
	std::vector<prosper::DeviceSize> offsets;
	std::optional<prosper::IndexBufferInfo> indexBufferInfo {};
	if(GetRenderBufferTargets(mesh, pipelineIdx, buffers, offsets, indexBufferInfo) == false)
		return nullptr;
	buffers.insert(buffers.begin(), CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer().get()); // Instance buffer
	offsets.insert(offsets.begin(), 0);
	auto *dummyBuffer = get_cengine()->GetRenderContext().GetDummyBuffer().get();
	for(auto it = buffers.begin(); it != buffers.end(); ++it) {
		auto *buf = *it;
		*it = buf ? buf : dummyBuffer;
	}
	return GetContext().CreateRenderBuffer(static_cast<const prosper::GraphicsPipelineCreateInfo &>(*GetPipelineCreateInfo(pipelineIdx)), buffers, offsets, indexBufferInfo);
}

/////////////

void ShaderGameWorld::RecordSceneFlags(rendering::ShaderProcessor &shaderProcessor, SceneFlags sceneFlags) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(ScenePushConstants, flags), sizeof(sceneFlags), &sceneFlags);
}

bool ShaderGameWorld::RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const
{
	return shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, layout, entityInstanceDescriptorSetIndex, *renderC.GetRenderDescriptorSet());
}

bool ShaderGameWorld::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, material::CMaterial &mat) const
{
	auto descSetGroup = mat.GetDescriptorSetGroup(const_cast<ShaderGameWorld &>(*this));
	// if(descSetGroup == nullptr)
	// 	descSetGroup = const_cast<pragma::ShaderGameWorld*>(this)->InitializeMaterialDescriptorSet(mat,false); // Attempt to initialize on the fly (TODO: Is this thread safe?)
	if(descSetGroup == nullptr)
		return false;
	auto dsMatIdx = GetMaterialDescriptorSetIndex();
	if(!dsMatIdx)
		return false;
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), *dsMatIdx, *descSetGroup->GetDescriptorSet(0));
	return true;
}

void ShaderGameWorld::RecordClipPlane(rendering::ShaderProcessor &shaderProcessor, const Vector4 &clipPlane) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(pragma::ShaderGameWorld::ScenePushConstants, clipPlane), sizeof(clipPlane), &clipPlane);
}

void ShaderGameWorld::RecordDrawOrigin(rendering::ShaderProcessor &shaderProcessor, const Vector4 &drawOrigin) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(pragma::ShaderGameWorld::ScenePushConstants, drawOrigin), sizeof(drawOrigin), &drawOrigin);
}

void ShaderGameWorld::RecordDepthBias(rendering::ShaderProcessor &shaderProcessor, const Vector2 &depthBias) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(pragma::ShaderGameWorld::ScenePushConstants, depthBias), sizeof(depthBias), &depthBias);
}

void ShaderGameWorld::RecordVertexAnimationOffset(rendering::ShaderProcessor &shaderProcessor, uint32_t vertexAnimationOffset) const
{
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, offsetof(pragma::ShaderGameWorld::ScenePushConstants, vertexAnimInfo), sizeof(vertexAnimationOffset),
	  &vertexAnimationOffset);
}
