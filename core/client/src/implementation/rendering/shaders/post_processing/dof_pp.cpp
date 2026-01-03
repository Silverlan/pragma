// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.pp_dof;

using namespace pragma;

decltype(ShaderPPDoF::DESCRIPTOR_SET_TEXTURE) ShaderPPDoF::DESCRIPTOR_SET_TEXTURE = {shaderPPBase::get_descriptor_set_texture()};
decltype(ShaderPPDoF::DESCRIPTOR_SET_DEPTH_BUFFER) ShaderPPDoF::DESCRIPTOR_SET_DEPTH_BUFFER = {
  "DEPTH_BUFFER",
  {prosper::DescriptorSetInfo::Binding {"MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderPPDoF::ShaderPPDoF(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBase(context, identifier, "programs/post_processing/depth_of_field") { SetBaseShader<prosper::ShaderCopyImage>(); }

void ShaderPPDoF::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
void ShaderPPDoF::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();
	AddDefaultVertexAttributes();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_DEPTH_BUFFER);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}
bool ShaderPPDoF::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetDepth, const PushConstants &pushConstants) const
{
	return RecordPushConstants(bindState, pushConstants) && RecordBindDescriptorSets(bindState, {&descSetDepth}, DESCRIPTOR_SET_DEPTH_BUFFER.setIndex) && ShaderPPBase::RecordDraw(bindState, descSetTexture);
}
void ShaderPPDoF::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPPDoF>({{{prosper::Format::R16G16B16A16_UNorm, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::ColorAttachmentOptimal}}},
	  outRenderPass, pipelineIdx);
}
