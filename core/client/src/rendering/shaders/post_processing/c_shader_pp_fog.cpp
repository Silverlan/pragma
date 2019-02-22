#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_shader_copy_image.hpp>
#include <prosper_util.hpp>

using namespace pragma;

decltype(ShaderPPFog::DESCRIPTOR_SET_TEXTURE) ShaderPPFog::DESCRIPTOR_SET_TEXTURE = {ShaderPPBase::DESCRIPTOR_SET_TEXTURE};
decltype(ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER) ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Depth Buffer
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderPPFog::DESCRIPTOR_SET_CAMERA) ShaderPPFog::DESCRIPTOR_SET_CAMERA = {&ShaderScene::DESCRIPTOR_SET_CAMERA};
decltype(ShaderPPFog::DESCRIPTOR_SET_FOG) ShaderPPFog::DESCRIPTOR_SET_FOG = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Fog
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderPPFog::ShaderPPFog(prosper::Context &context,const std::string &identifier)
	: ShaderPPBase(context,identifier,"screen/fs_pp_fog")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderPPFog::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddDefaultVertexAttributes(pipelineInfo);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_DEPTH_BUFFER);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_FOG);
}

bool ShaderPPFog::Draw(Anvil::DescriptorSet &descSetTexture,Anvil::DescriptorSet &descSetDepth,Anvil::DescriptorSet &descSetCamera,Anvil::DescriptorSet &descSetFog)
{
	return RecordBindDescriptorSets({&descSetDepth,&descSetCamera,&descSetFog},DESCRIPTOR_SET_DEPTH_BUFFER.setIndex) &&
		ShaderPPBase::Draw(descSetTexture);
}


 // prosper TODO
#if 0
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/uniformbinding.h"

extern DLLCLIENT CGame *c_game;

using namespace Shader;

LINK_SHADER_TO_CLASS(PPFog,pp_fog);

PPFog::PPFog()
	: Screen("pp_fog","screen/vs_screen_uv","screen/fs_pp_fog")
{}

void PPFog::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	//Screen::InitializePipelineLayout(context,setLayouts,pushConstants);

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Texture
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Depth Buffer
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Dummy
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Render Settings
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Dummy
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Fog
	}));
}

void PPFog::BindScene(const Vulkan::CommandBufferObject *drawCmd,const Scene &scene)
{
	auto &descSet = scene.GetCameraDescriptorSetGraphics();
	drawCmd->BindDescriptorSet(umath::to_integral(DescSet::RenderSettings),GetPipeline()->GetPipelineLayout(),descSet);
}

void PPFog::Draw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descTexture,const Vulkan::DescriptorSet &descDepth,const Vulkan::DescriptorSet &descFog)
{
	auto &layout = GetPipeline()->GetPipelineLayout();
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::Fog),layout,descFog);
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::DepthBuffer),layout,descDepth);
	Screen::Draw(cmdBuffer,descTexture);
}

#endif
