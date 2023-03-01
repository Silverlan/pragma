/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_dof.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <shader/prosper_shader_t.hpp>

using namespace pragma;

decltype(ShaderPPDoF::DESCRIPTOR_SET_TEXTURE) ShaderPPDoF::DESCRIPTOR_SET_TEXTURE = {ShaderPPBase::DESCRIPTOR_SET_TEXTURE};
decltype(ShaderPPDoF::DESCRIPTOR_SET_DEPTH_BUFFER) ShaderPPDoF::DESCRIPTOR_SET_DEPTH_BUFFER = {{prosper::DescriptorSetInfo::Binding {// Depth Buffer
  prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}}};
ShaderPPDoF::ShaderPPDoF(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBase(context, identifier, "pfm/post_processing/fs_depth_of_field") { SetBaseShader<prosper::ShaderCopyImage>(); }

void ShaderPPDoF::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	AddDefaultVertexAttributes(pipelineInfo);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_DEPTH_BUFFER);
	AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
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
