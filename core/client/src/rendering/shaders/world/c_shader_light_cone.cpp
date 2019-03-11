#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_light_cone.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <datasystem_color.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

decltype(ShaderLightCone::DESCRIPTOR_SET_DEPTH_MAP) ShaderLightCone::DESCRIPTOR_SET_DEPTH_MAP = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Depth Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderLightCone::ShaderLightCone(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase(context,identifier,"effects/vs_light_cone","effects/fs_light_cone")
{
	SetBaseShader<ShaderTextured3D>();
}

bool ShaderLightCone::BindSceneCamera(const Scene &scene,bool bView)
{
	if(ShaderTextured3DBase::BindSceneCamera(scene,bView) == false)
		return false;
	auto *descSetDepth = scene.GetDepthDescriptorSet();
	if(descSetDepth == nullptr)
		return false;
	return RecordBindDescriptorSet(*descSetDepth,DESCRIPTOR_SET_DEPTH_MAP.setIndex);
}

bool ShaderLightCone::BindMaterialParameters(CMaterial &mat)
{
	if(ShaderTextured3DBase::BindMaterialParameters(mat) == false)
		return false;
	auto &data = mat.GetDataBlock();
	auto coneLength = 100.f;
	if(data != nullptr)
		coneLength = data->GetFloat("cone_height");
	return RecordPushConstants(
		PushConstants{coneLength},
		sizeof(ShaderTextured3DBase::PushConstants)
	);
}

void ShaderLightCone::InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderTextured3DBase::PushConstants) +sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT);
}
void ShaderLightCone::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_DEPTH_MAP);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,Anvil::CullModeFlagBits::NONE);
	pipelineInfo.toggle_depth_writes(false);
}

 // prosper TODO
#if 0
#include "c_shader_light_cone.h"
#include <datasystem_color.h>

extern DLLCLIENT CGame *c_game;

using namespace Shader;

LINK_SHADER_TO_CLASS(LightCone,light_cone);

LightCone::LightCone()
	: TexturedBase3D("light_cone","effects/vs_light_cone","effects/fs_light_cone")
{
	SetUseBloomAttachment(true);
}

void LightCone::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	TexturedBase3D::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,TexturedBase3D::PUSH_CONSTANT_COUNT,(sizeof(float) *5) /sizeof(float)});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Depth Map
	}));
}

bool LightCone::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	if(TexturedBase3D::BeginDraw(cmdBuffer,shaderPipeline) == false)
		return false;
	auto &scene = c_game->GetRenderScene();
	scene->ResolveDepthTexture(cmdBuffer);
	auto &tex = scene->GetRenderDepthBuffer();
	auto &descSetDepth = scene->GetRenderDepthDescriptorSet();
	tex->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

	auto &layout = GetPipeline()->GetPipelineLayout();
	cmdBuffer->BindDescriptorSet(static_cast<uint32_t>(DescSet::DepthMap),layout,descSetDepth);
	return true;
}

void LightCone::BindMaterialParameters(Material *mat)
{
	TexturedBase3D::BindMaterialParameters(mat);

	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();

	auto *data = mat->GetDataBlock();
	auto coneLength = 100.f;
	Vector4 coneColor {1.f,1.f,1.f,1.f};

	if(data != nullptr)
	{
		coneLength = data->GetFloat("cone_height");
		auto *dataCol = data->GetRawType<DataColor>("cone_color");
		if(dataCol != nullptr)
		{
			auto col = dataCol->GetValue();
			coneColor = {
				col.r /255.f,
				col.g /255.f,
				col.b /255.f,
				col.a /255.f
			};
		}
	}

	DataStream ds {};
	ds<<coneColor<<coneLength;
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,PUSH_CONSTANT_COUNT,ds->GetDataSize() /4,ds->GetData());
}

void LightCone::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	TexturedBase3D::SetupPipeline(pipelineIdx,info);
	auto *rasterizationState = const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState);
	rasterizationState->cullMode = vk::CullModeFlagBits::eNone;

	auto *depthStencilState = const_cast<vk::PipelineDepthStencilStateCreateInfo*>(info.pDepthStencilState);
	depthStencilState->depthWriteEnable = false;
}

void LightCone::BindMaterial(Material *mat)
{
	TexturedBase3D::BindMaterial(mat);
	BindMaterialParameters(mat);
}
#endif