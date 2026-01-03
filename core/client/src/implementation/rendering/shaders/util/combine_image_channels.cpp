// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.combine_image_channels;

import :client_state;

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
		maxExtents.width = math::max(maxExtents.width, ext.width);
		maxExtents.height = math::max(maxExtents.height, ext.height);
	}
	auto rt = CreateRenderTarget(context, maxExtents);
	if(!rt)
		return nullptr;

	auto dsg = CreateDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE.setIndex);
	auto &ds = *dsg->GetDescriptorSet();
	ds.SetBindingTexture(channelR, math::to_integral(TextureBinding::ChannelR));
	ds.SetBindingTexture(channelG, math::to_integral(TextureBinding::ChannelG));
	ds.SetBindingTexture(channelB, math::to_integral(TextureBinding::ChannelB));
	ds.SetBindingTexture(channelA, math::to_integral(TextureBinding::ChannelA));

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
	CreateCachedRenderPass<ShaderCombineImageChannels>(std::vector<prosper::util::RenderPassCreateInfo::AttachmentInfo> {{prosper::Format::R8G8B8A8_UNorm}}, outRenderPass, pipelineIdx);
}
