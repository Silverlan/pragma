#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader_depth_combine.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(DepthCombine,depthcombine);

DepthCombine::DepthCombine()
	: Screen("depthcombine","screen/vs_screen_uv","screen/fs_depth_combine")
{
	SetUseColorAttachments(false);
}

void DepthCombine::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange>&)
{
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT},
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}
	}));
}

void DepthCombine::InitializeRenderPasses() {m_renderPasses = {m_context->GenerateRenderPass(Anvil::Format::D32_SFLOAT)};}
#endif