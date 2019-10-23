#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_water.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_shader_copy_image.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

decltype(ShaderPPWater::DESCRIPTOR_SET_TEXTURE) ShaderPPWater::DESCRIPTOR_SET_TEXTURE = {&ShaderPPFog::DESCRIPTOR_SET_TEXTURE};
decltype(ShaderPPWater::DESCRIPTOR_SET_DEPTH_BUFFER) ShaderPPWater::DESCRIPTOR_SET_DEPTH_BUFFER = {&ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER};
decltype(ShaderPPWater::DESCRIPTOR_SET_REFRACTION_MAP) ShaderPPWater::DESCRIPTOR_SET_REFRACTION_MAP = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Refraction Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderPPWater::DESCRIPTOR_SET_CAMERA) ShaderPPWater::DESCRIPTOR_SET_CAMERA = {&ShaderScene::DESCRIPTOR_SET_CAMERA};
decltype(ShaderPPWater::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderPPWater::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderPPWater::DESCRIPTOR_SET_FOG) ShaderPPWater::DESCRIPTOR_SET_FOG = {&ShaderPPFog::DESCRIPTOR_SET_FOG};
ShaderPPWater::ShaderPPWater(prosper::Context &context,const std::string &identifier)
	: ShaderPPBase(context,identifier,"screen/fs_pp_water")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderPPWater::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_DEPTH_BUFFER);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_REFRACTION_MAP);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_FOG);
	AddDefaultVertexAttributes(pipelineInfo);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);
}

std::shared_ptr<prosper::DescriptorSetGroup> ShaderPPWater::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto &dev = c_engine->GetDevice();
	auto *dudvMap = mat.GetTextureInfo("dudvmap");
	if(dudvMap == nullptr || dudvMap->texture == nullptr)
		return nullptr;
	auto descSetGroup = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_REFRACTION_MAP);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	auto texture = std::static_pointer_cast<Texture>(dudvMap->texture);
	if(texture->HasValidVkTexture())
		prosper::util::set_descriptor_set_binding_texture(descSet,*texture->GetVkTexture(),0u);
	return descSetGroup;
}

bool ShaderPPWater::BindRefractionMaterial(CMaterial &mat)
{
	auto descSetGroup = mat.GetDescriptorSetGroup(*this);
	if(descSetGroup == nullptr)
		descSetGroup = InitializeMaterialDescriptorSet(mat); // Attempt to initialize on the fly
	if(descSetGroup == nullptr)
		return false;
	return RecordBindDescriptorSet(*(*descSetGroup)->get_descriptor_set(0u),DESCRIPTOR_SET_REFRACTION_MAP.setIndex);
}

bool ShaderPPWater::Draw(
	Anvil::DescriptorSet &descSetTexture,Anvil::DescriptorSet &descSetDepth,Anvil::DescriptorSet &descSetCamera,
	Anvil::DescriptorSet &descSetTime,Anvil::DescriptorSet &descSetFog,const Vector4 &clipPlane
)
{
	return RecordBindDescriptorSet(descSetDepth,DESCRIPTOR_SET_DEPTH_BUFFER.setIndex) &&
		RecordBindDescriptorSets({&descSetCamera,&descSetTime,&descSetFog},DESCRIPTOR_SET_CAMERA.setIndex) &&
		RecordPushConstants(clipPlane) &&
		ShaderPPBase::Draw(descSetTexture);
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/post_processing/c_shader_pp_water.hpp"
#include "pragma/rendering/uniformbinding.h"

extern DLLCLIENT CGame *c_game;

using namespace Shader;

LINK_SHADER_TO_CLASS(PPWater,pp_water);

PPWater::PPWater()
	: Screen("pp_water","screen/vs_screen_uv","screen/fs_pp_water")
{}

void PPWater::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	//Screen::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::FRAGMENT_BIT,0,4
	});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Texture
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Depth Buffer
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,umath::to_integral(Binding::RefractionMap),1,Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT)); // Refraction Map
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Camera
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Render Settings
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Dummy
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,umath::to_integral(Binding::Time),1,Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT)); // Time
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Fog
	}));
}

void PPWater::InitializeShaderPipelines(const Vulkan::Context &context)
{
	Screen::InitializeShaderPipelines(context);

	auto *pipeline = GetPipeline();

	auto &timeBuffer = *c_game->GetUniformBlockSwapBuffer(UniformBinding::Time);
	std::vector<std::shared_ptr<Vulkan::impl::BufferBase>> buffers = {
		timeBuffer
	};
	pipeline->SetBuffer(umath::to_integral(DescSet::Time),buffers);
}

bool PPWater::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	if(Screen::BeginDraw(cmdBuffer,shaderPipeline) == false)
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

void PPWater::BindScene(const Vulkan::CommandBufferObject *drawCmd,const Scene &scene)
{
	auto &descSet = scene.GetCameraDescriptorSetGraphics();
	drawCmd->BindDescriptorSet(umath::to_integral(DescSet::Camera),GetPipeline()->GetPipelineLayout(),descSet);
}

void PPWater::BindMaterial(const Vulkan::CommandBufferObject *drawCmd,const CMaterial &mat)
{
	auto &descSet = mat.GetDescriptorSet();
	drawCmd->BindDescriptorSet(umath::to_integral(DescSet::RefractionMap),GetPipeline()->GetPipelineLayout(),descSet);
}

void PPWater::Draw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descTexture,const Vulkan::DescriptorSet &descDepth,const Vulkan::DescriptorSet &descFog,const Vector4 &clipPlane)
{
	auto &layout = GetPipeline()->GetPipelineLayout();
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,sizeof(clipPlane) /sizeof(float),&clipPlane);
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::DepthBuffer),layout,descDepth);
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::Fog),layout,descFog);
	Screen::Draw(cmdBuffer,descTexture);
}
#endif
