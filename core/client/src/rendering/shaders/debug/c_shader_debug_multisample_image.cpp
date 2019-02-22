#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/rendering/shaders/debug/c_shader_debug_multisample_image.hpp"
#include "pragma/rendering/uniformbinding.h"

extern DLLCLIENT CGame *c_game;

using namespace Shader;

LINK_SHADER_TO_CLASS(DebugMultisampleImage,debug_multisample_image);

DebugMultisampleImage::DebugMultisampleImage(const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: Screen(identifier,vsShader,fsShader)
{}

DebugMultisampleImage::DebugMultisampleImage()
	: DebugMultisampleImage("debug_multisample_image","screen/vs_screen_uv","debug/fs_debug_multisample_image")
{}

void DebugMultisampleImage::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	//Screen::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,3});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Texture
	}));
}

void DebugMultisampleImage::Draw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descTexture,uint32_t width,uint32_t height,uint32_t sampleCount)
{
	auto &pipeline = *GetPipeline();
	auto &layout = pipeline.GetPipelineLayout();
	std::array<uint32_t,3> pushConstants = {width,height,sampleCount};
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,pushConstants.size(),pushConstants.data());
	Screen::Draw(cmdBuffer,descTexture);
}

/////////////////////////////////////////

LINK_SHADER_TO_CLASS(DebugMultisampleDepth,debug_multisample_depth);

DebugMultisampleDepth::DebugMultisampleDepth()
	: DebugMultisampleImage("debug_multisample_depth","screen/vs_screen_uv","debug/fs_debug_multisample_depth")
{}
#endif