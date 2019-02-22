#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader_refraction.hpp"
#include "pragma/rendering/uniformbinding.h"
#include <random>

extern DLLCLIENT CGame *c_game;

using namespace Shader;

LINK_SHADER_TO_CLASS(Refraction,refraction);

Refraction::Refraction(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ScreenClip(identifier,vsShader,fsShader,gsShader)
{}

Refraction::Refraction()
	: Refraction("refraction","screen/vs_screen_clip","screen/fs_refraction")
{}

void Refraction::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	ScreenClip::InitializePipelineLayout(context,setLayouts,pushConstants);
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,umath::to_integral(Binding::RefractionMap),1,Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT)); // Refraction Map
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,umath::to_integral(Binding::Time),1,Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT)); // Time
}

bool Refraction::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	if(ScreenClip::BeginDraw(cmdBuffer,shaderPipeline) == false)
		return false;
	auto &context = *m_context.get();
	auto *pipeline = GetPipeline();
	auto &layout = pipeline->GetPipelineLayout();

	// Bind static buffers
	auto &bufStatic = *pipeline->GetDescriptorBuffer(umath::to_integral(DescSet::Time));
	auto &descSetStatic = *bufStatic->GetDescriptorSet(context.GetFrameSwapIndex());
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::Time),layout,descSetStatic);
	return true;
}

void Refraction::BindMaterial(const Vulkan::CommandBufferObject *drawCmd,const CMaterial &mat)
{
	auto &descSet = mat.GetDescriptorSet();
	drawCmd->BindDescriptorSet(umath::to_integral(DescSet::RefractionMap),GetPipeline()->GetPipelineLayout(),descSet);
}

void Refraction::InitializeShaderPipelines(const Vulkan::Context &context)
{
	ScreenClip::InitializeShaderPipelines(context);

	auto *pipeline = GetPipeline();

	auto &timeBuffer = *c_game->GetUniformBlockSwapBuffer(UniformBinding::Time);
	std::vector<std::shared_ptr<Vulkan::impl::BufferBase>> buffers = {
		timeBuffer
	};
	pipeline->SetBuffer(umath::to_integral(DescSet::Time),buffers);
}

void Refraction::InitializeRenderPasses()
{
	m_renderPasses.push_back({m_context->GenerateRenderPass(Anvil::Format::R8G8B8A8_UNORM)});
}
#endif