/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <shader/prosper_shader_t.hpp>

using namespace pragma;

decltype(ShaderPPFXAA::DESCRIPTOR_SET_TEXTURE) ShaderPPFXAA::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURES",
  {prosper::DescriptorSetInfo::Binding {"SCENE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"PRE_TONEMAPPING", // Pre-tonemapped image (required for correct transparency)
      prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderPPFXAA::ShaderPPFXAA(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBase(context, identifier, "programs/post_processing/fxaa/fxaa") { SetBaseShader<prosper::ShaderCopyImage>(); }

void ShaderPPFXAA::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
void ShaderPPFXAA::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();
	AddDefaultVertexAttributes();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}
bool ShaderPPFXAA::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, const PushConstants &pushConstants) const { return RecordPushConstants(bindState, pushConstants) && ShaderPPBase::RecordDraw(bindState, descSetTexture); }
void ShaderPPFXAA::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPPFXAA>({{{prosper::Format::R8G8B8A8_UNorm, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::ColorAttachmentOptimal}}},
	  outRenderPass, pipelineIdx);
}
