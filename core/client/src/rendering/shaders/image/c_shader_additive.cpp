#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/rendering/shaders/image/c_shader_additive.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(Additive,additive);

Additive::Additive()
	: Screen("additive","screen/vs_screen_uv","screen/fs_additive")
{}

void Additive::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::FRAGMENT_BIT,1
	});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT},
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}
	}));
}

bool Additive::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,float addScale)
{
	auto r = Screen::BeginDraw(cmdBuffer);
	if(r == false)
		return false;
	auto &pipeline = *GetPipeline();
	cmdBuffer->PushConstants(pipeline.GetPipelineLayout(),Anvil::ShaderStageFlagBits::FRAGMENT_BIT,addScale);
	return r;
}
#endif