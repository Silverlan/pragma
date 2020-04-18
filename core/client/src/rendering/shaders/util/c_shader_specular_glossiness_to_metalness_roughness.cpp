/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/util/c_shader_specular_glossiness_to_metalness_roughness.hpp"
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

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;


decltype(pragma::ShaderSpecularGlossinessToMetalnessRoughness::DESCRIPTOR_SET_TEXTURE) pragma::ShaderSpecularGlossinessToMetalnessRoughness::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Diffuse Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Specular glossiness map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Ambient occlusion Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
pragma::ShaderSpecularGlossinessToMetalnessRoughness::ShaderSpecularGlossinessToMetalnessRoughness(prosper::Context &context,const std::string &identifier)
	: ShaderBaseImageProcessing{context,identifier,"util/fs_specular_glossiness_to_metalness_roughness.gls"}
{}

std::optional<pragma::ShaderSpecularGlossinessToMetalnessRoughness::MetalnessRoughnessImageSet> pragma::ShaderSpecularGlossinessToMetalnessRoughness::ConvertToMetalnessRoughness(
	prosper::Context &context,prosper::Texture *diffuseMap,prosper::Texture *specularGlossinessMap,const PushConstants &pushConstantData,prosper::Texture *aoMap
)
{
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = Anvil::Format::R8G8B8A8_UNORM;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	imgCreateInfo.tiling = Anvil::ImageTiling::OPTIMAL;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;

	auto fGetWhiteTex = [&context]() -> prosper::Texture* {
		TextureManager::LoadInfo loadInfo {};
		loadInfo.flags = TextureLoadFlags::LoadInstantly;
		std::shared_ptr<void> tex = nullptr;
		if(static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(context,"white",loadInfo,&tex) == false)
			return nullptr;
		return std::static_pointer_cast<Texture>(tex)->GetVkTexture().get();
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

	auto &dev = context.GetDevice();

	auto extents = imgDiffuse->GetExtents();
	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;
	auto imgAlbedo = prosper::util::create_image(context.GetDevice(),imgCreateInfo);

	extents = imgSpecularGlossiness->GetExtents();
	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;
	auto imgRMA = prosper::util::create_image(context.GetDevice(),imgCreateInfo);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	auto texAlbedo = prosper::util::create_texture(dev,{},imgAlbedo,&imgViewCreateInfo);
	auto texRMA = prosper::util::create_texture(dev,{},imgRMA,&imgViewCreateInfo);

	auto rtAlbedo = prosper::util::create_render_target(dev,{texAlbedo},GetRenderPass());
	auto rtRMA = prosper::util::create_render_target(dev,{texRMA},GetRenderPass());

	auto dsg = CreateDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE.setIndex);
	auto &ds = *dsg->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_texture(ds,*diffuseMap,umath::to_integral(TextureBinding::DiffuseMap));
	prosper::util::set_descriptor_set_binding_texture(ds,*specularGlossinessMap,umath::to_integral(TextureBinding::SpecularGlossinessMap));
	prosper::util::set_descriptor_set_binding_texture(ds,*aoMap,umath::to_integral(TextureBinding::AmbientOcclusionMap));

	auto setupCmd = context.GetSetupCommandBuffer();
	auto pushConstants = pushConstantData;
	pushConstants.pass = Pass::Albedo;
	if(prosper::util::record_begin_render_pass(**setupCmd,*rtAlbedo))
	{
		if(BeginDraw(setupCmd))
		{
			if(RecordPushConstants(pushConstants))
				Draw(*ds);
			EndDraw();
		}
		prosper::util::record_end_render_pass(**setupCmd);
	}

	context.FlushSetupCommandBuffer();
	setupCmd = context.GetSetupCommandBuffer();
	pushConstants.pass = Pass::RMA;
	if(prosper::util::record_begin_render_pass(**setupCmd,*rtRMA))
	{
		if(BeginDraw(setupCmd))
		{
			if(RecordPushConstants(pushConstants))
				Draw(*ds);
			EndDraw();
		}
		prosper::util::record_end_render_pass(**setupCmd);
	}
	context.FlushSetupCommandBuffer();

	return MetalnessRoughnessImageSet{
		texAlbedo->GetImage(),
		texRMA->GetImage()
	};
}

void pragma::ShaderSpecularGlossinessToMetalnessRoughness::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddDefaultVertexAttributes(pipelineInfo);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);
	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
}

void pragma::ShaderSpecularGlossinessToMetalnessRoughness::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<pragma::ShaderSpecularGlossinessToMetalnessRoughness>(
		std::vector<prosper::util::RenderPassCreateInfo::AttachmentInfo>{
			{Anvil::Format::R8G8B8A8_UNORM} // Albedo / RMA
	},outRenderPass,pipelineIdx);
}

