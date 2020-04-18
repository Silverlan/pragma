#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_brdf_convolution.hpp"
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_util_square_shape.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;


ShaderBRDFConvolution::ShaderBRDFConvolution(prosper::Context &context,const std::string &identifier)
	: ShaderBaseImageProcessing{context,identifier,"screen/fs_brdf_convolution.gls"}
{}
void ShaderBRDFConvolution::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderBRDFConvolution>({{prosper::util::RenderPassCreateInfo::AttachmentInfo{Anvil::Format::R16G16_SFLOAT}}},outRenderPass,pipelineIdx);
}
void ShaderBRDFConvolution::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddDefaultVertexAttributes(pipelineInfo);
}
std::shared_ptr<prosper::Texture> ShaderBRDFConvolution::CreateBRDFConvolutionMap(uint32_t resolution)
{
	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.format = Anvil::Format::R16G16_SFLOAT;
	createInfo.width = resolution;
	createInfo.height = resolution;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.tiling = Anvil::ImageTiling::OPTIMAL;
	createInfo.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	auto img = prosper::util::create_image(dev,createInfo);

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.minFilter = Anvil::Filter::LINEAR;
	samplerCreateInfo.magFilter = Anvil::Filter::LINEAR;
	auto tex = prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
	auto rt = prosper::util::create_render_target(dev,{tex},GetRenderPass());

	auto vertBuffer = prosper::util::get_square_vertex_buffer(dev);
	auto uvBuffer = prosper::util::get_square_uv_buffer(dev);
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	auto success = false;
	if(prosper::util::record_begin_render_pass(**setupCmd,*rt))
	{
		if(BeginDraw(setupCmd))
		{
			if(
				RecordBindVertexBuffers({&vertBuffer->GetAnvilBuffer(),&uvBuffer->GetAnvilBuffer()}) &&
				RecordDraw(prosper::util::get_square_vertex_count())
			)
				success = true;
		}
		prosper::util::record_end_render_pass(**setupCmd);
	}
	c_engine->FlushSetupCommandBuffer();
	return success ? tex : nullptr;
}

