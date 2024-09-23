/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/info/c_shader_velocity_buffer.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

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
