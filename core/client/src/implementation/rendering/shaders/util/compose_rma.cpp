// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.compose_rma;

import :client_state;
import :engine;
import :game;

decltype(pragma::ShaderComposeRMA::DESCRIPTOR_SET_TEXTURE) pragma::ShaderComposeRMA::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURES",
  {prosper::DescriptorSetInfo::Binding {"ROUGHNESS", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"METALNESS", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"AMBIENT_OCCLUSION", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
pragma::ShaderComposeRMA::ShaderComposeRMA(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/util/compose_rma"} {}

std::shared_ptr<prosper::IImage> pragma::ShaderComposeRMA::ComposeRMA(prosper::IPrContext &context, prosper::Texture *roughnessMap, prosper::Texture *metalnessMap, prosper::Texture *aoMap, Flags flags)
{
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::SampledBit;

	auto fGetWhiteTex = [&context]() -> prosper::Texture * {
		auto tex = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager()).GetTextureManager().LoadAsset("white");
		if(tex == nullptr)
			return nullptr;
		return tex->GetVkTexture().get();
	};

	if(roughnessMap == nullptr)
		roughnessMap = fGetWhiteTex();
	if(metalnessMap == nullptr)
		metalnessMap = fGetWhiteTex();
	if(aoMap == nullptr)
		aoMap = fGetWhiteTex();
	if(roughnessMap == nullptr || metalnessMap == nullptr || aoMap == nullptr)
		return nullptr;

	auto &imgRoughness = roughnessMap->GetImage();
	auto &imgMetalness = metalnessMap->GetImage();
	auto &imgAo = aoMap->GetImage();

	auto extents = imgRoughness.GetExtents();
	extents.width = std::max({extents.width, imgMetalness.GetExtents().width, imgAo.GetExtents().width});
	extents.height = std::max({extents.height, imgMetalness.GetExtents().height, imgAo.GetExtents().height});

	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;

	auto imgRMA = context.CreateImage(imgCreateInfo);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	auto texRMA = context.CreateTexture({}, *imgRMA, imgViewCreateInfo);
	auto rt = context.CreateRenderTarget({texRMA}, GetRenderPass());

	auto dsg = CreateDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE.setIndex);
	auto &ds = *dsg->GetDescriptorSet();
	ds.SetBindingTexture(*roughnessMap, math::to_integral(TextureBinding::RoughnessMap));
	ds.SetBindingTexture(*metalnessMap, math::to_integral(TextureBinding::MetalnessMap));
	ds.SetBindingTexture(*aoMap, math::to_integral(TextureBinding::AmbientOcclusionMap));

	auto &setupCmd = context.GetSetupCommandBuffer();
	if(setupCmd->RecordBeginRenderPass(*rt)) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState)) {
			PushConstants pushConstants {};
			pushConstants.flags = flags;
			if(RecordPushConstants(bindState, pushConstants))
				RecordDraw(bindState, ds);
			RecordEndDraw(bindState);
		}
		setupCmd->RecordEndRenderPass();
	}
	context.FlushSetupCommandBuffer();

	return texRMA->GetImage().shared_from_this();
}
bool pragma::ShaderComposeRMA::InsertAmbientOcclusion(prosper::IPrContext &context, const std::string &rmaInputPath, prosper::IImage &aoImg, const std::string *optRmaOutputPath)
{
	auto &texManager = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager()).GetTextureManager();
	auto rmaTexInfo = texManager.LoadAsset(rmaInputPath);
	if(rmaTexInfo == nullptr || rmaTexInfo->HasValidVkTexture() == false)
		return false;

	prosper::util::TextureCreateInfo texCreateInfo {};
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto aoTex = context.CreateTexture(texCreateInfo, aoImg, imgViewCreateInfo, samplerCreateInfo);

	auto texRMA = (rmaTexInfo->IsError() == false) ? rmaTexInfo->GetVkTexture() : nullptr;
	auto newRMA = ComposeRMA(context, texRMA.get(), texRMA.get(), aoTex.get());

	image::TextureInfo imgWriteInfo {};
	imgWriteInfo.alphaMode = image::TextureInfo::AlphaMode::None;
	imgWriteInfo.containerFormat = image::TextureInfo::ContainerFormat::DDS;
	imgWriteInfo.flags = image::TextureInfo::Flags::GenerateMipmaps;
	imgWriteInfo.inputFormat = image::TextureInfo::InputFormat::R8G8B8A8_UInt;
	imgWriteInfo.outputFormat = image::TextureInfo::OutputFormat::ColorMap;

	auto rmaOutputPath = optRmaOutputPath ? *optRmaOutputPath : rmaInputPath;

	std::string materialsRootDir = "materials/";
	auto matName = materialsRootDir + rmaOutputPath;
	ufile::remove_extension_from_filename(matName);

	Con::COUT << "Writing RMA texture file '" << rmaOutputPath << "'..." << Con::endl;
	// TODO: RMA should overwrite the existing one
	return get_cgame()->SaveImage(*newRMA, "addons/converted/" + matName, imgWriteInfo);
}
bool pragma::ShaderComposeRMA::InsertAmbientOcclusion(prosper::IPrContext &context, const std::string &rmaInputPath, image::ImageBuffer &imgBuffer, const std::string *optRmaOutputPath)
{
	auto aoImg = context.CreateImage(imgBuffer);
	return InsertAmbientOcclusion(context, rmaInputPath, *aoImg, optRmaOutputPath);
}

void pragma::ShaderComposeRMA::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
}

void pragma::ShaderComposeRMA::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddDefaultVertexAttributes();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

void pragma::ShaderComposeRMA::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderComposeRMA>(
	  std::vector<prosper::util::RenderPassCreateInfo::AttachmentInfo> {
	    {prosper::Format::R8G8B8A8_UNorm} // RMA
	  },
	  outRenderPass, pipelineIdx);
}
