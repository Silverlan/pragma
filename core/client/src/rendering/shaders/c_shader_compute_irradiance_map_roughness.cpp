#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_compute_irradiance_map_roughness.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <pragma/util/stb_image.h>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_image_view.hpp>
#include <prosper_framebuffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
decltype(ShaderComputeIrradianceMapRoughness::DESCRIPTOR_SET_IRRADIANCE) ShaderComputeIrradianceMapRoughness::DESCRIPTOR_SET_IRRADIANCE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderComputeIrradianceMapRoughness::DESCRIPTOR_SET_ROUGHNESS) ShaderComputeIrradianceMapRoughness::DESCRIPTOR_SET_ROUGHNESS = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderComputeIrradianceMapRoughness::ShaderComputeIrradianceMapRoughness(prosper::Context &context,const std::string &identifier)
	: ShaderCubemap{context,identifier,"screen/fs_compute_irradiance_map_roughness"}
{}

void ShaderComputeIrradianceMapRoughness::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderCubemap::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_IRRADIANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_ROUGHNESS);
}

void ShaderComputeIrradianceMapRoughness::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderComputeIrradianceMapRoughness>({{prosper::util::RenderPassCreateInfo::AttachmentInfo{Anvil::Format::R16G16B16A16_SFLOAT}}},outRenderPass,pipelineIdx);
}

std::shared_ptr<prosper::Texture> ShaderComputeIrradianceMapRoughness::ComputeRoughness(prosper::Texture &cubemap,uint32_t resolution)
{
	auto &cubemapImg = cubemap.GetImage();
	auto w = resolution;
	auto h = resolution;
	prosper::util::ImageCreateInfo::Flags flags = prosper::util::ImageCreateInfo::Flags::FullMipmapChain;
	auto img = CreateCubeMap(w,h,flags);

	auto &dev = c_engine->GetDevice();
	constexpr uint8_t layerCount = 6u;
	constexpr uint8_t maxMipLevels = 5u;
	struct MipLevelFramebuffer
	{
		std::shared_ptr<prosper::ImageView> imageView;
		std::shared_ptr<prosper::Framebuffer> framebuffer;
	};
	std::array<std::array<MipLevelFramebuffer,maxMipLevels>,layerCount> imgViews {};
	for(auto layerId=decltype(imgViews.size()){0u};layerId<imgViews.size();++layerId)
	{
		auto &layerViews = imgViews.at(layerId);
		for(auto mipLevel=decltype(layerViews.size()){0u};mipLevel<layerViews.size();++mipLevel)
		{
			auto &mipLevelFramebuffer = layerViews.at(mipLevel);
			prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
			imgViewCreateInfo.baseLayer = layerId;
			imgViewCreateInfo.baseMipmap = mipLevel;
			imgViewCreateInfo.format = img->GetFormat();
			imgViewCreateInfo.levelCount = 1u;
			imgViewCreateInfo.mipmapLevels = 1u;

			mipLevelFramebuffer.imageView = prosper::util::create_image_view(dev,imgViewCreateInfo,img);
			uint32_t wMipmap,hMipmap;
			prosper::util::calculate_mipmap_size(w,h,&wMipmap,&hMipmap,mipLevel);
			std::vector<prosper::ImageView*> imgViewAttachments {mipLevelFramebuffer.imageView.get()};
			mipLevelFramebuffer.framebuffer = prosper::util::create_framebuffer(dev,wMipmap,hMipmap,1u,imgViewAttachments);
		}
	}

	// Shader input
	auto dsg = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_IRRADIANCE);
	prosper::util::set_descriptor_set_binding_texture(*dsg->GetDescriptorSet(),cubemap,0u);

	PushConstants pushConstants {};
	pushConstants.projection = GetProjectionMatrix(w /static_cast<float>(h));

	// Generate cube
	uint32_t numVerts;
	auto vertexBuffer = CreateCubeMesh(numVerts);

	// Roughness data buffer
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	bufCreateInfo.size = sizeof(RoughnessData);
	bufCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT;
	auto buf = prosper::util::create_buffer(dev,bufCreateInfo);
	buf->SetPermanentlyMapped(true);

	auto dsgRoughness = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_ROUGHNESS);
	prosper::util::set_descriptor_set_binding_uniform_buffer(*dsgRoughness->GetDescriptorSet(),*buf,0);

	// Shader execution
	auto success = true;
	// Note: Computing the roughness takes a long time, which triggers the TDR timeout.
	// So, instead, we render each triangle of the cube separately for each individual layer.
	auto rp = GetRenderPass();
	RoughnessData roughnessData {};
	roughnessData.resolution = resolution;
	for(uint8_t mipLevel=0u;mipLevel<maxMipLevels;++mipLevel)
	{
		roughnessData.roughness = static_cast<float>(mipLevel) /static_cast<float>(maxMipLevels -1u);
		for(uint8_t layerId=0u;layerId<layerCount;++layerId)
		{
			for(uint32_t i=0u;i<numVerts;i+=3)
			{
				auto &setupCmd = c_engine->GetSetupCommandBuffer();
				ScopeGuard sgCmd {[this]() {
					GetContext().FlushSetupCommandBuffer();
				}};
				prosper::util::record_update_buffer(**setupCmd,*buf,0,roughnessData);
				prosper::util::record_buffer_barrier(
					**setupCmd,*buf,
					Anvil::PipelineStageFlagBits::HOST_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
					Anvil::AccessFlagBits::HOST_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
				);

				prosper::util::ImageSubresourceRange range {};
				range.baseArrayLayer = layerId;
				range.layerCount = 1u;
				range.baseMipLevel = mipLevel;
				range.levelCount = 1u;
				auto &mipLevelFramebuffer = imgViews.at(layerId).at(mipLevel);
				auto &fb = *mipLevelFramebuffer.framebuffer;

				if(
					prosper::util::record_image_barrier(**setupCmd,**img,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,range) == false ||
					prosper::util::record_begin_render_pass(**setupCmd,*img,*rp,fb) == false
					)
				{
					success = false;
					goto endLoop;
				}

				if(BeginDrawViewport(setupCmd,fb.GetWidth(),fb.GetHeight()) == true)
				{
					pushConstants.view = GetViewMatrix(layerId);
					success = RecordPushConstants(pushConstants) && RecordBindDescriptorSets({(*dsg)->get_descriptor_set(0u),(*dsgRoughness)->get_descriptor_set(0u)}) &&
						RecordBindVertexBuffer(**vertexBuffer) && RecordDraw(3u,1u,i);
					EndDraw();
				}
				success = success && prosper::util::record_end_render_pass(**setupCmd);
				success = success && prosper::util::record_post_render_pass_image_barrier(**setupCmd,**img,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,range);

				if(success == false)
					goto endLoop;
			}
		}
	}
endLoop:
	if(success == false)
		return nullptr;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo{};
	InitializeSamplerCreateInfo(flags,samplerCreateInfo);
	prosper::util::TextureCreateInfo texCreateInfo {};
	InitializeTextureCreateInfo(texCreateInfo);
	auto tex = prosper::util::create_texture(dev,texCreateInfo,img,&imgViewCreateInfo,&samplerCreateInfo);
	return success ? tex : nullptr;

}
#pragma optimize("",on)
