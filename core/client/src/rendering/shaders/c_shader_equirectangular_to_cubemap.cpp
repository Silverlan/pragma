#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_equirectangular_to_cubemap.hpp"
#include "pragma/rendering/pbr/stb_image.h"
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
decltype(ShaderEquirectangularToCubemap::DESCRIPTOR_SET_EQUIRECTANGULAR_TEXTURE) ShaderEquirectangularToCubemap::DESCRIPTOR_SET_EQUIRECTANGULAR_TEXTURE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderEquirectangularToCubemap::ShaderEquirectangularToCubemap(prosper::Context &context,const std::string &identifier)
	: ShaderCubemap{context,identifier,"screen/fs_equirectangular_to_cubemap"}
{}

void ShaderEquirectangularToCubemap::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderCubemap::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_EQUIRECTANGULAR_TEXTURE);
}

void ShaderEquirectangularToCubemap::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderEquirectangularToCubemap>({{prosper::util::RenderPassCreateInfo::AttachmentInfo{Anvil::Format::R32G32B32A32_SFLOAT}}},outRenderPass,pipelineIdx);
}

std::shared_ptr<prosper::Texture> ShaderEquirectangularToCubemap::LoadEquirectangularImage(const std::string &fileName,uint32_t resolution)
{
	auto f = FileManager::OpenFile(fileName.c_str(),"rb");
	if(f == nullptr)
		return nullptr;
	return LoadEquirectangularImage(f,resolution);
}
std::shared_ptr<prosper::Texture> ShaderEquirectangularToCubemap::LoadEquirectangularImage(VFilePtr f,uint32_t resolution)
{
	int width,height,nrComponents;
	stbi_io_callbacks ioCallbacks {};
	ioCallbacks.read = [](void *user,char *data,int size) -> int {
		return static_cast<VFilePtrInternal*>(user)->Read(data,size);
	};
	ioCallbacks.skip = [](void *user,int n) -> void {
		auto *f = static_cast<VFilePtrInternal*>(user);
		f->Seek(f->Tell() +n);
	};
	ioCallbacks.eof = [](void *user) -> int {
		return static_cast<VFilePtrInternal*>(user)->Eof();
	};
	std::unique_ptr<float,void(*)(float*)> data{stbi_loadf_from_callbacks(&ioCallbacks,f.get(),&width,&height,&nrComponents,4),[](float *data) {
		if(data)
			stbi_image_free(data);
	}};
	if(data == nullptr)
		return nullptr;

	prosper::util::ImageCreateInfo createInfo {};
	createInfo.width = width;
	createInfo.height = height;
	createInfo.format = Anvil::Format::R32G32B32A32_SFLOAT;
	createInfo.tiling = Anvil::ImageTiling::LINEAR;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	createInfo.usage = Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;

	auto &context = *c_engine;
	auto &dev = context.GetDevice();
	auto img = prosper::util::create_image(dev,createInfo,reinterpret_cast<uint8_t*>(data.get()));

	prosper::util::TextureCreateInfo texCreateInfo {};
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.minFilter = Anvil::Filter::LINEAR;
	samplerCreateInfo.magFilter = Anvil::Filter::LINEAR;
	samplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	auto equirectRadianceTex = prosper::util::create_texture(dev,texCreateInfo,img,&imgViewCreateInfo,&samplerCreateInfo);
	return EquirectangularTextureToCubemap(*equirectRadianceTex,resolution);
}

std::shared_ptr<prosper::Texture> ShaderEquirectangularToCubemap::EquirectangularTextureToCubemap(prosper::Texture &equirectangularTexture,uint32_t resolution)
{
	auto rt = CreateCubeMapRenderTarget(resolution,resolution,prosper::util::ImageCreateInfo::Flags::FullMipmapChain);

	// Shader input
	auto &dev = c_engine->GetDevice();
	auto dsg = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_EQUIRECTANGULAR_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(*(*dsg)->get_descriptor_set(0u),equirectangularTexture,0u);

	PushConstants pushConstants {};
	pushConstants.projection = GetProjectionMatrix(resolution /static_cast<float>(resolution));

	// Generate cube
	uint32_t numVerts;
	auto vertexBuffer = CreateCubeMesh(numVerts);

	// Shader execution
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	prosper::util::record_post_render_pass_image_barrier(**setupCmd,**rt->GetTexture()->GetImage(),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	auto success = true;
	for(uint8_t layerId=0u;layerId<6u;++layerId)
	{
		if(prosper::util::record_begin_render_pass(**setupCmd,*rt,layerId) == false)
		{
			success = false;
			break;
		}
		if(BeginDraw(setupCmd) == true)
		{
			pushConstants.view = GetViewMatrix(layerId);
			success = RecordPushConstants(pushConstants) && RecordBindDescriptorSet(*(*dsg)->get_descriptor_set(0u)) &&
				RecordBindVertexBuffer(**vertexBuffer) && RecordDraw(numVerts);
			EndDraw();
		}
		success = success && prosper::util::record_end_render_pass(**setupCmd);
		if(success == false)
			break;
	}
	prosper::util::record_post_render_pass_image_barrier(**setupCmd,**rt->GetTexture()->GetImage(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	prosper::util::record_generate_mipmaps(
		**setupCmd,**rt->GetTexture()->GetImage(),
		Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
		Anvil::AccessFlagBits::SHADER_READ_BIT,
		Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT
	);
	GetContext().FlushSetupCommandBuffer();
	return success ? rt->GetTexture() : nullptr;
}
#pragma optimize("",on)
