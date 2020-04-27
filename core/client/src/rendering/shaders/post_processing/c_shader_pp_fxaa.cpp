/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include <prosper_util.hpp>
#include <shader/prosper_shader_copy_image.hpp>

using namespace pragma;

decltype(ShaderPPFXAA::DESCRIPTOR_SET_TEXTURE) ShaderPPFXAA::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::DescriptorSetInfo::Binding {
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Pre-tonemapped image (required for correct transparency)
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderPPFXAA::ShaderPPFXAA(prosper::Context &context,const std::string &identifier)
	: ShaderPPBase(context,identifier,"screen/fxaa/fs_pp_fxaa")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderPPFXAA::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddDefaultVertexAttributes(pipelineInfo);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit);
}
bool ShaderPPFXAA::Draw(prosper::IDescriptorSet &descSetTexture,const PushConstants &pushConstants)
{
	return RecordPushConstants(pushConstants) &&
		ShaderPPBase::Draw(descSetTexture);
}
void ShaderPPFXAA::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPPFXAA>({{{
		prosper::Format::R8G8B8A8_UNorm,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::DontCare,
		prosper::AttachmentStoreOp::Store,prosper::SampleCountFlags::e1Bit,prosper::ImageLayout::ColorAttachmentOptimal
	}}},outRenderPass,pipelineIdx);
}
