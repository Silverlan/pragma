#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_convolute_cubemap_lighting.hpp"
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;


decltype(ShaderConvoluteCubemapLighting::DESCRIPTOR_SET_CUBEMAP_TEXTURE) ShaderConvoluteCubemapLighting::DESCRIPTOR_SET_CUBEMAP_TEXTURE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderConvoluteCubemapLighting::ShaderConvoluteCubemapLighting(prosper::Context &context,const std::string &identifier)
	: ShaderCubemap{context,identifier,"screen/fs_convolute_cubemap_lighting"}
{}

void ShaderConvoluteCubemapLighting::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderCubemap::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CUBEMAP_TEXTURE);
}

void ShaderConvoluteCubemapLighting::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderConvoluteCubemapLighting>({{prosper::util::RenderPassCreateInfo::AttachmentInfo{Anvil::Format::R16G16B16A16_SFLOAT}}},outRenderPass,pipelineIdx);
}

std::shared_ptr<prosper::Texture> ShaderConvoluteCubemapLighting::ConvoluteCubemapLighting(prosper::Texture &cubemap,uint32_t resolution)
{
	auto &cubemapImg = cubemap.GetImage();
	auto w = resolution;
	auto h = resolution;
	auto rt = CreateCubeMapRenderTarget(w,h);

	// Shader input
	auto &dev = c_engine->GetDevice();
	auto dsg = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_CUBEMAP_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(*dsg->GetDescriptorSet(),cubemap,0u);

	PushConstants pushConstants {};
	pushConstants.projection = GetProjectionMatrix(w /static_cast<float>(h));

	// Generate cube
	uint32_t numVerts;
	auto vertexBuffer = CreateCubeMesh(numVerts);

	// Shader execution
	auto success = true;
	// Note: Convoluting the lighting takes a long time, which triggers the TDR timeout.
	// So, instead, we render each triangle of the cube separately for each individual layer.
	for(uint8_t layerId=0u;layerId<6u;++layerId)
	{
		for(uint32_t i=0u;i<numVerts;i+=3)
		{
			auto &setupCmd = c_engine->GetSetupCommandBuffer();
			ScopeGuard sgCmd {[this]() {
				GetContext().FlushSetupCommandBuffer();
			}};
			prosper::util::ImageSubresourceRange range {};
			range.baseArrayLayer = layerId;
			range.layerCount = 1u;
			auto &img = rt->GetTexture()->GetImage();
			if(
				prosper::util::record_image_barrier(**setupCmd,**img,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,range) == false ||
				prosper::util::record_begin_render_pass(**setupCmd,*rt,layerId) == false
			)
			{
				success = false;
				goto endLoop;
			}
			if(BeginDraw(setupCmd) == true)
			{
				pushConstants.view = GetViewMatrix(layerId);
				success = RecordPushConstants(pushConstants) && RecordBindDescriptorSet(*(*dsg)->get_descriptor_set(0u)) &&
					RecordBindVertexBuffer(**vertexBuffer) && RecordDraw(3u,1u,i);
				EndDraw();
			}
			success = success && prosper::util::record_end_render_pass(**setupCmd);
			success = success && prosper::util::record_post_render_pass_image_barrier(**setupCmd,**img,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,range);

			if(success == false)
				goto endLoop;
		}
	}
endLoop:
	return success ? rt->GetTexture() : nullptr;
}

