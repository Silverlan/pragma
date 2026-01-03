// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.brdf_convolution;

import :engine;

using namespace pragma;

ShaderBRDFConvolution::ShaderBRDFConvolution(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/lighting/brdf_convolution"} {}
void ShaderBRDFConvolution::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderBRDFConvolution>({{prosper::util::RenderPassCreateInfo::AttachmentInfo {prosper::Format::R16G16_SFloat}}}, outRenderPass, pipelineIdx);
}
void ShaderBRDFConvolution::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
void ShaderBRDFConvolution::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddDefaultVertexAttributes();
}
std::shared_ptr<prosper::Texture> ShaderBRDFConvolution::CreateBRDFConvolutionMap(uint32_t resolution)
{
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.format = prosper::Format::R16G16_SFloat;
	createInfo.width = resolution;
	createInfo.height = resolution;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.tiling = prosper::ImageTiling::Optimal;
	createInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit;
	auto img = get_cengine()->GetRenderContext().CreateImage(createInfo);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.minFilter = prosper::Filter::Linear;
	samplerCreateInfo.magFilter = prosper::Filter::Linear;
	auto tex = get_cengine()->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	auto rt = get_cengine()->GetRenderContext().CreateRenderTarget({tex}, GetRenderPass());

	auto vertBuffer = get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	auto uvBuffer = get_cengine()->GetRenderContext().GetCommonBufferCache().GetSquareUvBuffer();
	auto &setupCmd = get_cengine()->GetSetupCommandBuffer();
	auto success = false;
	if(setupCmd->RecordBeginRenderPass(*rt)) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState)) {
			if(RecordBindVertexBuffers(bindState, {vertBuffer.get(), uvBuffer.get()}) && ShaderGraphics::RecordDraw(bindState, prosper::CommonBufferCache::GetSquareVertexCount()))
				success = true;
		}
		setupCmd->RecordEndRenderPass();
	}
	get_cengine()->FlushSetupCommandBuffer();
	return success ? tex : nullptr;
}
