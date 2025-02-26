/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/rendering/shaders/util/c_shader_combine_image_channels.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_util.hpp>
#include <prosper_context.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <util_image_buffer.hpp>
#include <util_texture_info.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_texture.hpp>
#include <image/prosper_image.hpp>
#include <image/prosper_sampler.hpp>
#include <sharedutils/util_file.h>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>
#include <texturemanager/texture.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

decltype(pragma::ShaderCombineImageChannels::DESCRIPTOR_SET_TEXTURE) pragma::ShaderCombineImageChannels::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURES",
  {
    prosper::DescriptorSetInfo::Binding {"CHANNEL_R", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"CHANNEL_G", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"CHANNEL_B", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"CHANNEL_A", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  },
};
pragma::ShaderCombineImageChannels::ShaderCombineImageChannels(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/util/combine_image_channels"} {}

std::shared_ptr<prosper::RenderTarget> pragma::ShaderCombineImageChannels::CreateRenderTarget(prosper::IPrContext &context, prosper::Extent2D extents) const
{
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit;
	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;
	auto img = context.CreateImage(imgCreateInfo);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	auto texAlbedo = context.CreateTexture({}, *img, imgViewCreateInfo);
	return context.CreateRenderTarget({texAlbedo}, GetRenderPass());
}

std::shared_ptr<prosper::Texture> pragma::ShaderCombineImageChannels::CombineImageChannels(prosper::IPrContext &context, prosper::Texture &channelR, prosper::Texture &channelG, prosper::Texture &channelB, prosper::Texture &channelA, const PushConstants &pushConstants)
{
	std::array<prosper::Extent2D, 4> extents = {channelR.GetImage().GetExtents(), channelG.GetImage().GetExtents(), channelB.GetImage().GetExtents(), channelA.GetImage().GetExtents()};
	prosper::Extent2D maxExtents {};
	for(auto &ext : extents) {
		maxExtents.width = umath::max(maxExtents.width, ext.width);
		maxExtents.height = umath::max(maxExtents.height, ext.height);
	}
	auto rt = CreateRenderTarget(context, maxExtents);
	if(!rt)
		return nullptr;

	auto dsg = CreateDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE.setIndex);
	auto &ds = *dsg->GetDescriptorSet();
	ds.SetBindingTexture(channelR, umath::to_integral(TextureBinding::ChannelR));
	ds.SetBindingTexture(channelG, umath::to_integral(TextureBinding::ChannelG));
	ds.SetBindingTexture(channelB, umath::to_integral(TextureBinding::ChannelB));
	ds.SetBindingTexture(channelA, umath::to_integral(TextureBinding::ChannelA));

	auto setupCmd = context.GetSetupCommandBuffer();
	if(setupCmd->RecordBeginRenderPass(*rt)) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState)) {
			if(RecordPushConstants(bindState, pushConstants))
				RecordDraw(bindState, ds);
			RecordEndDraw(bindState);
		}
		setupCmd->RecordEndRenderPass();
	}
	context.FlushSetupCommandBuffer();

	return rt->GetTexture().shared_from_this();
}

void pragma::ShaderCombineImageChannels::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
}

void pragma::ShaderCombineImageChannels::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddDefaultVertexAttributes();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

void pragma::ShaderCombineImageChannels::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<pragma::ShaderCombineImageChannels>(std::vector<prosper::util::RenderPassCreateInfo::AttachmentInfo> {{prosper::Format::R8G8B8A8_UNorm}}, outRenderPass, pipelineIdx);
}
