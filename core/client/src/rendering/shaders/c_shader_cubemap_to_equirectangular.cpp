#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_cubemap_to_equirectangular.hpp"
#include <prosper_util_square_shape.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
ShaderCubemapToEquirectangular::ShaderCubemapToEquirectangular(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderBaseImageProcessing{context,identifier,"screen/fs_cubemap_to_equirectangular"}
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}

void ShaderCubemapToEquirectangular::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto format = (pipelineIdx == 0) ? Anvil::Format::R16G16B16A16_SFLOAT : Anvil::Format::R8G8B8A8_UNORM;
	CreateCachedRenderPass<ShaderCubemapToEquirectangular>({{prosper::util::RenderPassCreateInfo::AttachmentInfo{format}}},outRenderPass,pipelineIdx);
}

std::shared_ptr<prosper::Image> ShaderCubemapToEquirectangular::CreateEquirectangularMap(uint32_t width,uint32_t height,prosper::util::ImageCreateInfo::Flags flags) const
{
	// TODO: Add support for HDR!
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.format = Anvil::Format::R8G8B8A8_UNORM; // Anvil::Format::R16G16B16A16_SFLOAT;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.tiling = Anvil::ImageTiling::OPTIMAL;
	createInfo.flags |= flags;
	createInfo.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	createInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;

	auto &dev = c_engine->GetDevice();
	return prosper::util::create_image(dev,createInfo);
}

std::shared_ptr<prosper::RenderTarget> ShaderCubemapToEquirectangular::CreateEquirectangularRenderTarget(uint32_t width,uint32_t height,prosper::util::ImageCreateInfo::Flags flags) const
{
	auto &dev = c_engine->GetDevice();
	auto img = CreateEquirectangularMap(width,height,flags);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	//InitializeSamplerCreateInfo(flags,samplerCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	//InitializeTextureCreateInfo(texCreateInfo);
	auto tex = prosper::util::create_texture(dev,texCreateInfo,img,&imgViewCreateInfo,&samplerCreateInfo);

	prosper::util::RenderTargetCreateInfo rtCreateInfo {};
	//rtCreateInfo.useLayerFramebuffers = true;
	return prosper::util::create_render_target(dev,{tex},GetRenderPass(),rtCreateInfo);
}

std::shared_ptr<prosper::Texture> ShaderCubemapToEquirectangular::CubemapToEquirectangularTexture(prosper::Texture &cubemap,uint32_t width,uint32_t height)
{
	auto rt = CreateEquirectangularRenderTarget(width,height,prosper::util::ImageCreateInfo::Flags::FullMipmapChain);
	auto format = cubemap.GetImage()->GetFormat();

	// Shader input
	auto &dev = c_engine->GetDevice();
	auto dsg = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(*dsg->GetDescriptorSet(),cubemap,0u);

	// Shader execution
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	prosper::util::record_post_render_pass_image_barrier(**setupCmd,**rt->GetTexture()->GetImage(),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	auto success = true;

	auto vertBuffer = prosper::util::get_square_vertex_buffer(dev);
	auto uvBuffer = prosper::util::get_square_uv_buffer(dev);
	auto numVerts = prosper::util::get_square_vertex_count();

	if(prosper::util::record_begin_render_pass(**setupCmd,*rt) == false)
		success = false;
	else
	{
		auto pipelineIdx = Pipeline::RGBA16;
		if(format == Anvil::Format::R8G8B8A8_UNORM)
			pipelineIdx = Pipeline::RGBA8;
		if(BeginDraw(setupCmd,umath::to_integral(pipelineIdx)) == true)
		{
			success = RecordBindDescriptorSet(*(*dsg)->get_descriptor_set(0u)) &&
				RecordBindVertexBuffers({&vertBuffer->GetAnvilBuffer(),&uvBuffer->GetAnvilBuffer()}) && RecordDraw(numVerts);
			EndDraw();
		}
		success = success && prosper::util::record_end_render_pass(**setupCmd);
	}

	auto &img = **rt->GetTexture()->GetImage();
	prosper::util::record_post_render_pass_image_barrier(**setupCmd,img,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	prosper::util::record_generate_mipmaps(
		**setupCmd,img,
		Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
		Anvil::AccessFlagBits::SHADER_READ_BIT,
		Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT
	);
	GetContext().FlushSetupCommandBuffer();
	return success ? rt->GetTexture() : nullptr;
}
#pragma optimize("",on)
