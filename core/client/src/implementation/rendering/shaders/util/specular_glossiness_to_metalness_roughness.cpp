// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.specular_glossiness_to_metalness_roughness;

import :client_state;

decltype(pragma::ShaderSpecularGlossinessToMetalnessRoughness::DESCRIPTOR_SET_TEXTURE) pragma::ShaderSpecularGlossinessToMetalnessRoughness::DESCRIPTOR_SET_TEXTURE = {
  "TEXTURES",
  {prosper::DescriptorSetInfo::Binding {"DIFFUSE", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"SPECULAR_GLOSSINESS", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"AMBIENT_OCCLUSION", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
pragma::ShaderSpecularGlossinessToMetalnessRoughness::ShaderSpecularGlossinessToMetalnessRoughness(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/util/specular_glossiness_to_metalness_roughness"} {}

std::optional<pragma::ShaderSpecularGlossinessToMetalnessRoughness::MetalnessRoughnessImageSet> pragma::ShaderSpecularGlossinessToMetalnessRoughness::ConvertToMetalnessRoughness(prosper::IPrContext &context, prosper::Texture *diffuseMap, prosper::Texture *specularGlossinessMap,
  const PushConstants &pushConstantData, prosper::Texture *aoMap)
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

	if(diffuseMap == nullptr)
		diffuseMap = fGetWhiteTex();
	if(specularGlossinessMap == nullptr)
		specularGlossinessMap = fGetWhiteTex();
	if(aoMap == nullptr)
		aoMap = fGetWhiteTex();
	if(diffuseMap == nullptr || specularGlossinessMap == nullptr || aoMap == nullptr)
		return {};

	auto &imgDiffuse = diffuseMap->GetImage();
	auto &imgSpecularGlossiness = specularGlossinessMap->GetImage();
	auto &imgAo = aoMap->GetImage();

	auto extents = imgDiffuse.GetExtents();
	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;
	auto imgAlbedo = context.CreateImage(imgCreateInfo);

	extents = imgSpecularGlossiness.GetExtents();
	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;
	auto imgRMA = context.CreateImage(imgCreateInfo);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	auto texAlbedo = context.CreateTexture({}, *imgAlbedo, imgViewCreateInfo);
	auto texRMA = context.CreateTexture({}, *imgRMA, imgViewCreateInfo);

	auto rtAlbedo = context.CreateRenderTarget({texAlbedo}, GetRenderPass());
	auto rtRMA = context.CreateRenderTarget({texRMA}, GetRenderPass());

	auto dsg = CreateDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE.setIndex);
	auto &ds = *dsg->GetDescriptorSet();
	ds.SetBindingTexture(*diffuseMap, math::to_integral(TextureBinding::DiffuseMap));
	ds.SetBindingTexture(*specularGlossinessMap, math::to_integral(TextureBinding::SpecularGlossinessMap));
	ds.SetBindingTexture(*aoMap, math::to_integral(TextureBinding::AmbientOcclusionMap));

	auto setupCmd = context.GetSetupCommandBuffer();
	auto pushConstants = pushConstantData;
	pushConstants.pass = Pass::Albedo;
	if(setupCmd->RecordBeginRenderPass(*rtAlbedo)) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState)) {
			if(RecordPushConstants(bindState, pushConstants))
				RecordDraw(bindState, ds);
			RecordEndDraw(bindState);
		}
		setupCmd->RecordEndRenderPass();
	}

	context.FlushSetupCommandBuffer();
	setupCmd = context.GetSetupCommandBuffer();
	pushConstants.pass = Pass::RMA;
	if(setupCmd->RecordBeginRenderPass(*rtRMA)) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(RecordBeginDraw(bindState)) {
			if(RecordPushConstants(bindState, pushConstants))
				RecordDraw(bindState, ds);
			RecordEndDraw(bindState);
		}
		setupCmd->RecordEndRenderPass();
	}
	context.FlushSetupCommandBuffer();

	return MetalnessRoughnessImageSet {texAlbedo->GetImage().shared_from_this(), texRMA->GetImage().shared_from_this()};
}

void pragma::ShaderSpecularGlossinessToMetalnessRoughness::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
}

void pragma::ShaderSpecularGlossinessToMetalnessRoughness::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddDefaultVertexAttributes();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

void pragma::ShaderSpecularGlossinessToMetalnessRoughness::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderSpecularGlossinessToMetalnessRoughness>(
	  std::vector<prosper::util::RenderPassCreateInfo::AttachmentInfo> {
	    {prosper::Format::R8G8B8A8_UNorm} // Albedo / RMA
	  },
	  outRenderPass, pipelineIdx);
}
