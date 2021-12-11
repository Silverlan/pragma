/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/c_settings.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/console/convars.h>
#include <prosper_util.hpp>
#include <shader/prosper_shader_copy_image.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

using namespace pragma;

decltype(ShaderPPHDR::DESCRIPTOR_SET_TEXTURE) ShaderPPHDR::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::DescriptorSetInfo::Binding { // Texture
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Bloom
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		/*prosper::DescriptorSetInfo::Binding { // Glow
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}*/
	}
};
decltype(ShaderPPHDR::RENDER_PASS_FORMAT) ShaderPPHDR::RENDER_PASS_FORMAT = prosper::Format::R8G8B8A8_UNorm;
ShaderPPHDR::ShaderPPHDR(prosper::IPrContext &context,const std::string &identifier)
	: ShaderPPBase(context,identifier,"screen/fs_pp_hdr")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderPPHDR::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderPPHDR>({{
		{
			RENDER_PASS_FORMAT,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::DontCare,
			prosper::AttachmentStoreOp::Store,prosper::SampleCountFlags::e1Bit,prosper::ImageLayout::TransferSrcOptimal
		}
	}},outRenderPass,pipelineIdx);
}

void ShaderPPHDR::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddDefaultVertexAttributes(pipelineInfo);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit);

	auto &settings = client->GetGameWorldShaderSettings();
	AddSpecializationConstant(pipelineInfo,prosper::ShaderStageFlags::FragmentBit,0u /* constantId */,static_cast<uint32_t>(settings.bloomEnabled));
	AddSpecializationConstant(pipelineInfo,prosper::ShaderStageFlags::FragmentBit,1u /* constantId */,static_cast<uint32_t>(settings.fxaaEnabled));
}

bool ShaderPPHDR::Draw(prosper::IDescriptorSet &descSetTexture,pragma::rendering::ToneMapping toneMapping,float exposure,float bloomScale,float glowScale,bool flipVertically)
{
	return RecordPushConstants(PushConstants{exposure,bloomScale,glowScale,toneMapping,flipVertically ? 1u : 0u}) &&
		ShaderPPBase::Draw(descSetTexture) == true;
}
