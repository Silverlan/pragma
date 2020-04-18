#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_light_cone.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <datasystem_color.h>
#include <pragma/entities/entity_component_system_t.hpp>

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
	// SetBaseShader<ShaderTextured3DBase>();
	umath::set_flag(m_stateFlags,StateFlags::ShouldUseLightMap,false);
}

bool ShaderLightCone::BindSceneCamera(const pragma::rendering::RasterizationRenderer &renderer,bool bView)
{
	if(ShaderTextured3DBase::BindSceneCamera(renderer,bView) == false)
		return false;
	auto *descSetDepth = renderer.GetDepthDescriptorSet();
	if(descSetDepth == nullptr)
		return false;
	return RecordBindDescriptorSet(*descSetDepth,DESCRIPTOR_SET_DEPTH_MAP.setIndex);
}

bool ShaderLightCone::BindEntity(CBaseEntity &ent)
{
	if(ShaderTextured3DBase::BindEntity(ent) == false)
		return false;
	auto pSpotVolComponent = ent.GetComponent<CLightSpotVolComponent>();
	auto lightIndex = -1;
	if(pSpotVolComponent.valid())
	{
		auto *entSpotlight = pSpotVolComponent->GetSpotlightTarget();
		if(entSpotlight != nullptr)
		{
			auto pLightComponent = entSpotlight->GetComponent<CLightComponent>();
			if(pLightComponent.valid())
			{
				auto &renderBuffer = pLightComponent->GetRenderBuffer();
				if(renderBuffer != nullptr)
					lightIndex = renderBuffer->GetBaseIndex();
			}
		}
	}
	m_boundLightIndex = lightIndex;
	return true;
}

std::shared_ptr<prosper::DescriptorSetGroup> ShaderLightCone::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto descSetGroup = prosper::util::create_descriptor_set_group(GetContext().GetDevice(),DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	return descSetGroup;
}

bool ShaderLightCone::Draw(CModelSubMesh &mesh)
{
	return RecordPushConstants( // Light cone shader doesn't use lightmaps, so we hijack the lightmapFlags push constant for our own purposes
		static_cast<uint32_t>(m_boundLightIndex),
		sizeof(ShaderTextured3DBase::PushConstants) +offsetof(PushConstants,boundLightIndex)
	) && ShaderTextured3DBase::Draw(mesh);
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
		sizeof(ShaderTextured3DBase::PushConstants) +offsetof(PushConstants,coneLength)
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
