// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.cubemap_to_equirectangular;

import :engine;

using namespace pragma;

ShaderCubemapToEquirectangular::ShaderCubemapToEquirectangular(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/util/cubemap_to_equirectangular"} { SetPipelineCount(math::to_integral(Pipeline::Count)); }

void ShaderCubemapToEquirectangular::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	auto format = (pipelineIdx == 0) ? prosper::Format::R16G16B16A16_SFloat : prosper::Format::R8G8B8A8_UNorm;
	CreateCachedRenderPass<ShaderCubemapToEquirectangular>({{prosper::util::RenderPassCreateInfo::AttachmentInfo {format}}}, outRenderPass, pipelineIdx);
}

void ShaderCubemapToEquirectangular::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderCubemapToEquirectangular::InitializeShaderResources()
{
	ShaderBaseImageProcessing::InitializeShaderResources();
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

std::shared_ptr<prosper::IImage> ShaderCubemapToEquirectangular::CreateEquirectangularMap(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags, bool hdr) const
{
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.format = hdr ? prosper::Format::R16G16B16A16_SFloat : prosper::Format::R8G8B8A8_UNorm;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.tiling = prosper::ImageTiling::Optimal;
	createInfo.flags |= flags;
	createInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	createInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;

	return get_cengine()->GetRenderContext().CreateImage(createInfo);
}

std::shared_ptr<prosper::RenderTarget> ShaderCubemapToEquirectangular::CreateEquirectangularRenderTarget(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags, bool hdr) const
{
	auto img = CreateEquirectangularMap(width, height, flags, hdr);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	//InitializeSamplerCreateInfo(flags,samplerCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	//InitializeTextureCreateInfo(texCreateInfo);
	auto tex = get_cengine()->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);

	prosper::util::RenderTargetCreateInfo rtCreateInfo {};
	//rtCreateInfo.useLayerFramebuffers = true;
	auto rt = get_cengine()->GetRenderContext().CreateRenderTarget({tex}, GetRenderPass(), rtCreateInfo);
	if(rt)
		rt->SetDebugName("equirectangular_render_target");
	return rt;
}

std::shared_ptr<prosper::Texture> ShaderCubemapToEquirectangular::CubemapToEquirectangularTexture(prosper::Texture &cubemap, uint32_t width, uint32_t height, math::Degree range, prosper::ImageLayout cubemapLayout)
{
	auto inputFormat = cubemap.GetImage().GetFormat();
	// TODO: If compressed, check if compressed HDR format
	auto hdr = prosper::util::is_16bit_format(inputFormat) || prosper::util::is_32bit_format(inputFormat) || prosper::util::is_compressed_format(inputFormat);
	auto rt = CreateEquirectangularRenderTarget(width, height, prosper::util::ImageCreateInfo::Flags::FullMipmapChain, hdr);
	auto format = cubemap.GetImage().GetFormat();

	// Shader input
	auto dsg = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(shaderCubemapToEquirectangular::get_descriptor_set_texture());
	dsg->GetDescriptorSet()->SetBindingTexture(cubemap, 0u);

	// Shader execution
	auto &setupCmd = get_cengine()->GetSetupCommandBuffer();
	if(cubemapLayout != prosper::ImageLayout::ShaderReadOnlyOptimal)
		setupCmd->RecordImageBarrier(cubemap.GetImage(), cubemapLayout, prosper::ImageLayout::ShaderReadOnlyOptimal);
	setupCmd->RecordImageBarrier(rt->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	auto success = true;

	auto vertBuffer = get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	auto uvBuffer = get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareUvBuffer();
	auto numVerts = prosper::CommonBufferCache::GetSquareVertexCount();

	if(setupCmd->RecordBeginRenderPass(*rt) == false)
		success = false;
	else {
		auto pipelineIdx = Pipeline::RGBA16;
		if(format == prosper::Format::R8G8B8A8_UNorm)
			pipelineIdx = Pipeline::RGBA8;

		PushConstants pushConstants {};
		pushConstants.xFactor = range / 360.f;

		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState, math::to_integral(pipelineIdx)) == true) {
			success = RecordPushConstants(bindState, pushConstants) && RecordBindDescriptorSet(bindState, *dsg->GetDescriptorSet()) && RecordBindVertexBuffers(bindState, {vertBuffer.get(), uvBuffer.get()}) && ShaderGraphics::RecordDraw(bindState, numVerts);
			RecordEndDraw(bindState);
		}
		success = success && setupCmd->RecordEndRenderPass();
	}

	auto &img = rt->GetTexture().GetImage();
	setupCmd->RecordPostRenderPassImageBarrier(img, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	setupCmd->RecordGenerateMipmaps(img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::AccessFlags::ShaderReadBit, prosper::PipelineStageFlags::FragmentShaderBit);
	if(cubemapLayout != prosper::ImageLayout::ShaderReadOnlyOptimal)
		setupCmd->RecordImageBarrier(cubemap.GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, cubemapLayout);
	GetContext().FlushSetupCommandBuffer();
	return success ? rt->GetTexture().shared_from_this() : nullptr;
}
