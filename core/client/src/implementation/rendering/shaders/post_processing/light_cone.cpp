// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :model;
import :rendering.shaders.pp_light_cone;

using namespace pragma;

decltype(ShaderPPLightCone::DESCRIPTOR_SET_TEXTURE) ShaderPPLightCone::DESCRIPTOR_SET_TEXTURE = {shaderPPBase::get_descriptor_set_texture()};
decltype(ShaderPPLightCone::DESCRIPTOR_SET_DEPTH_BUFFER) ShaderPPLightCone::DESCRIPTOR_SET_DEPTH_BUFFER = {
  "DEPTH_BUFFER",
  {prosper::DescriptorSetInfo::Binding {"MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderPPLightCone::DESCRIPTOR_SET_SCENE) ShaderPPLightCone::DESCRIPTOR_SET_SCENE = {&ShaderEntity::DESCRIPTOR_SET_SCENE};
decltype(ShaderPPLightCone::DESCRIPTOR_SET_INSTANCE) ShaderPPLightCone::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};

decltype(ShaderPPLightCone::VERTEX_BINDING_VERTEX) ShaderPPLightCone::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex, sizeof(rendering::VertexBufferData)};
decltype(ShaderPPLightCone::VERTEX_ATTRIBUTE_POSITION) ShaderPPLightCone::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION, VERTEX_BINDING_VERTEX};
decltype(ShaderPPLightCone::VERTEX_ATTRIBUTE_NORMAL) ShaderPPLightCone::VERTEX_ATTRIBUTE_NORMAL = {ShaderEntity::VERTEX_ATTRIBUTE_NORMAL, VERTEX_BINDING_VERTEX};

ShaderPPLightCone::ShaderPPLightCone(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBase(context, identifier, "programs/scene/light_cone/light_cone", "programs/scene/light_cone/light_cone") { SetBaseShader<prosper::ShaderCopyImage>(); }
void ShaderPPLightCone::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	pipelineInfo.ToggleDepthWrites(false);
	pipelineInfo.ToggleDepthTest(true, prosper::CompareOp::LessOrEqual);

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	auto blendOp = prosper::BlendOp::Add;
	auto colorComponents = prosper::ColorComponentFlags::RBit | prosper::ColorComponentFlags::GBit | prosper::ColorComponentFlags::BBit | prosper::ColorComponentFlags::ABit;
	pipelineInfo.SetColorBlendAttachmentProperties(1u, true, blendOp, blendOp, prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::DstAlpha, // color
	  prosper::BlendFactor::One, prosper::BlendFactor::OneMinusSrcAlpha,                                                                       // alpha
	  colorComponents);
	/*pipelineInfo.SetColorBlendAttachmentProperties(
		1u,true,prosper::BlendOp::Add,prosper::BlendOp::Add,
		prosper::BlendFactor::SrcAlpha,prosper::BlendFactor::OneMinusSrcAlpha,
		prosper::BlendFactor::SrcAlpha,prosper::BlendFactor::OneMinusSrcAlpha,
		prosper::ColorComponentFlags::RBit | prosper::ColorComponentFlags::GBit | prosper::ColorComponentFlags::BBit | prosper::ColorComponentFlags::ABit
	);*/
}
void ShaderPPLightCone::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(VERTEX_ATTRIBUTE_NORMAL);

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_DEPTH_BUFFER);
	AddDescriptorSetGroup(DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
}
void ShaderPPLightCone::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	auto sampleCount = prosper::SampleCountFlags::e1Bit;
	prosper::util::RenderPassCreateInfo rpCreateInfo {{{ShaderScene::RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal},
	  {// Bloom Attachment
	    ShaderScene::RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal},
	  {ShaderScene::RENDER_PASS_DEPTH_FORMAT, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::DepthStencilAttachmentOptimal}}};
	rpCreateInfo.subPasses.push_back(prosper::util::RenderPassCreateInfo::SubPass {std::vector<std::size_t> {0ull, 1ull}, true});
	CreateCachedRenderPass<ShaderPPLightCone>(rpCreateInfo, outRenderPass, pipelineIdx);
}
bool ShaderPPLightCone::RecordDraw(prosper::ShaderBindState &bindState, const geometry::CModelSubMesh &mesh, prosper::IDescriptorSet &descSetTex, prosper::IDescriptorSet &descSetDepth, prosper::IDescriptorSet &descSetInstance, prosper::IDescriptorSet &descSetCam) const
{
	auto &vkMesh = mesh.GetSceneMesh();
	if(!vkMesh)
		return false;
	auto &vertexBuf = vkMesh->GetVertexBuffer();
	auto &indexBuf = vkMesh->GetIndexBuffer();
	if(!vertexBuf || !indexBuf)
		return false;
	auto indexType = (mesh.GetIndexType() == geometry::IndexType::UInt16) ? prosper::IndexType::UInt16 : prosper::IndexType::UInt32;
	return RecordBindVertexBuffer(bindState, *vertexBuf) && RecordBindIndexBuffer(bindState, *indexBuf, indexType) && RecordBindDescriptorSets(bindState, {&descSetDepth}, DESCRIPTOR_SET_DEPTH_BUFFER.setIndex)
	  && RecordBindDescriptorSet(bindState, descSetTex, DESCRIPTOR_SET_TEXTURE.setIndex) && RecordBindDescriptorSet(bindState, descSetInstance, DESCRIPTOR_SET_INSTANCE.setIndex) && RecordBindDescriptorSet(bindState, descSetCam, DESCRIPTOR_SET_SCENE.setIndex)
	  && RecordDrawIndexed(bindState, mesh.GetIndexCount());
}
bool ShaderPPLightCone::RecordPushConstants(prosper::ShaderBindState &bindState, const PushConstants &pushConstants) const { return ShaderPPBase::RecordPushConstants(bindState, pushConstants); }
