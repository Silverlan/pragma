// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.equirectangular_to_cubemap;

import :engine;

using namespace pragma;

decltype(ShaderEquirectangularToCubemap::DESCRIPTOR_SET_EQUIRECTANGULAR_TEXTURE) ShaderEquirectangularToCubemap::DESCRIPTOR_SET_EQUIRECTANGULAR_TEXTURE = {
  "TEXTURE",
  {prosper::DescriptorSetInfo::Binding {"EQUIRECTANGULAR_TEXTURE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderEquirectangularToCubemap::ShaderEquirectangularToCubemap(prosper::IPrContext &context, const std::string &identifier) : ShaderCubemap {context, identifier, "programs/util/equirectangular_to_cubemap"} {}

void ShaderEquirectangularToCubemap::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderCubemap::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderEquirectangularToCubemap::InitializeShaderResources()
{
	ShaderCubemap::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_EQUIRECTANGULAR_TEXTURE);
}

void ShaderEquirectangularToCubemap::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderEquirectangularToCubemap>({{prosper::util::RenderPassCreateInfo::AttachmentInfo {prosper::Format::R16G16B16A16_SFloat}}}, outRenderPass, pipelineIdx);
}

std::shared_ptr<prosper::Texture> ShaderEquirectangularToCubemap::LoadEquirectangularImage(const std::string &fileName, uint32_t resolution)
{
	auto f = pragma::fs::open_file(fileName.c_str(), fs::FileMode::Read | fs::FileMode::Binary);
	if(f == nullptr)
		return nullptr;
	return LoadEquirectangularImage(f, resolution);
}
std::shared_ptr<prosper::Texture> ShaderEquirectangularToCubemap::LoadEquirectangularImage(fs::VFilePtr fp, uint32_t resolution)
{
	fs::File f {fp};
	auto imgBuffer = image::load_image(f, image::PixelFormat::Float);
	if(imgBuffer == nullptr)
		return nullptr;
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.width = imgBuffer->GetWidth();
	createInfo.height = imgBuffer->GetHeight();
	createInfo.format = prosper::Format::R32G32B32A32_SFloat;
	createInfo.tiling = prosper::ImageTiling::Linear;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
	createInfo.usage = prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::SampledBit;

	auto &context = get_cengine()->GetRenderContext();
	auto img = context.CreateImage(createInfo, reinterpret_cast<uint8_t *>(imgBuffer->GetData()));

	prosper::util::TextureCreateInfo texCreateInfo {};
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.minFilter = prosper::Filter::Linear;
	samplerCreateInfo.magFilter = prosper::Filter::Linear;
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	auto equirectRadianceTex = context.CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);
	return EquirectangularTextureToCubemap(*equirectRadianceTex, resolution);
}

std::shared_ptr<prosper::Texture> ShaderEquirectangularToCubemap::EquirectangularTextureToCubemap(prosper::Texture &equirectangularTexture, uint32_t resolution)
{
	auto rt = CreateCubeMapRenderTarget(resolution, resolution, prosper::util::ImageCreateInfo::Flags::FullMipmapChain);

	// Shader input
	auto dsg = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_EQUIRECTANGULAR_TEXTURE);
	dsg->GetDescriptorSet()->SetBindingTexture(equirectangularTexture, 0u);

	PushConstants pushConstants {};
	pushConstants.projection = GetProjectionMatrix(resolution / static_cast<float>(resolution));

	// Generate cube
	uint32_t numVerts;
	auto vertexBuffer = CreateCubeMesh(numVerts);

	// Shader execution
	auto &setupCmd = get_cengine()->GetSetupCommandBuffer();
	auto success = true;
	for(uint8_t layerId = 0u; layerId < 6u; ++layerId) {
		setupCmd->RecordImageBarrier(rt->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal, prosper::util::ImageSubresourceRange {layerId});
		if(setupCmd->RecordBeginRenderPass(*rt, layerId) == false) {
			success = false;
			break;
		}
		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState) == true) {
			pushConstants.view = GetViewMatrix(layerId);
			success = RecordPushConstants(bindState, pushConstants) && RecordBindDescriptorSet(bindState, *dsg->GetDescriptorSet()) && RecordBindVertexBuffer(bindState, *vertexBuffer) && RecordDraw(bindState, numVerts);
			RecordEndDraw(bindState);
		}
		success = success && setupCmd->RecordEndRenderPass();
		if(success == false)
			break;
	}
	auto &img = rt->GetTexture().GetImage();
	setupCmd->RecordGenerateMipmaps(img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::AccessFlags::ShaderReadBit, prosper::PipelineStageFlags::FragmentShaderBit);
	GetContext().FlushSetupCommandBuffer();
	return success ? rt->GetTexture().shared_from_this() : nullptr;
}
