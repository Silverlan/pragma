// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.velocity_buffer;

import :client_state;
import :engine;
import :game;

using namespace pragma;

decltype(ShaderVelocityBuffer::DESCRIPTOR_SET_MOTION_BLUR) ShaderVelocityBuffer::DESCRIPTOR_SET_MOTION_BLUR = {
  "MOTION_BLUR",
  {prosper::DescriptorSetInfo::Binding {"DATA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::VertexBit}},
};
decltype(ShaderVelocityBuffer::DESCRIPTOR_SET_BONE_BUFFER) ShaderVelocityBuffer::DESCRIPTOR_SET_BONE_BUFFER = {
  "BONES",
  {prosper::DescriptorSetInfo::Binding {"MATRIX_DATA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::VertexBit}},
};
ShaderVelocityBuffer::ShaderVelocityBuffer(prosper::IPrContext &context, const std::string &identifier) : ShaderPrepassBase {context, identifier, "world/vs_velocity_buffer", "world/fs_velocity_buffer"} {}

void ShaderVelocityBuffer::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	prosper::util::RenderPassCreateInfo::AttachmentInfo attInfo {prosper::Format::R32G32B32A32_SFloat, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Clear, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit,
	  prosper::ImageLayout::ColorAttachmentOptimal};
	auto depthAtt = get_depth_render_pass_attachment_info(GetSampleCount(pipelineIdx));
	depthAtt.loadOp = prosper::AttachmentLoadOp::Load;
	depthAtt.storeOp = prosper::AttachmentStoreOp::Store;
	CreateCachedRenderPass<ShaderVelocityBuffer>(prosper::util::RenderPassCreateInfo {std::vector<prosper::util::RenderPassCreateInfo::AttachmentInfo> {prosper::util::RenderPassCreateInfo::AttachmentInfo {attInfo}, depthAtt}}, outRenderPass, pipelineIdx);
}

void ShaderVelocityBuffer::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderPrepassBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	pipelineInfo.ToggleDepthWrites(false);
	pipelineInfo.ToggleDepthTest(true, prosper::CompareOp::LessOrEqual);
}

void ShaderVelocityBuffer::InitializeShaderResources()
{
	ShaderPrepassBase::InitializeShaderResources();

	AttachPushConstantRange(sizeof(PushConstants), sizeof(MotionBlurPushConstants), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit);
	AddDescriptorSetGroup(DESCRIPTOR_SET_MOTION_BLUR);
	AddDescriptorSetGroup(DESCRIPTOR_SET_BONE_BUFFER);
}
