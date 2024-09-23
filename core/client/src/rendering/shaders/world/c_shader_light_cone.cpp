/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_light_cone.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <datasystem_color.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <prosper_command_buffer.hpp>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderLightCone::DESCRIPTOR_SET_DEPTH_MAP) ShaderLightCone::DESCRIPTOR_SET_DEPTH_MAP = {
  "DEPTH_BUFFER",
  {prosper::DescriptorSetInfo::Binding {"MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderLightCone::ShaderLightCone(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "effects/vs_light_cone", "effects/fs_light_cone")
{
	// SetBaseShader<ShaderTextured3DBase>();
	umath::set_flag(m_sceneFlags, SceneFlags::LightmapsEnabled, false);
}

bool ShaderLightCone::RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const
{
	if(ShaderGameWorldLightingPass::RecordBindEntity(shaderProcessor, renderC, layout, entityInstanceDescriptorSetIndex) == false)
		return false;
	auto lightSpotVol = renderC.GetEntity().GetComponent<CLightSpotVolComponent>();
	if(lightSpotVol.expired())
		return false;
	auto &cmd = shaderProcessor.GetCommandBuffer();
	return false;
	/*return cmd.RecordPushConstants( // Light cone shader doesn't use lightmaps, so we hijack the lightmapFlags push constant for our own purposes
		static_cast<uint32_t>(m_boundLightIndex),
		sizeof(ShaderGameWorldLightingPass::PushConstants) +offsetof(PushConstants,boundLightIndex)
	);*/
}

#if 0
bool ShaderLightCone::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount)
{
	return RecordPushConstants( // Light cone shader doesn't use lightmaps, so we hijack the lightmapFlags push constant for our own purposes
		static_cast<uint32_t>(m_boundLightIndex),
		sizeof(ShaderGameWorldLightingPass::PushConstants) +offsetof(PushConstants,boundLightIndex)
	) && ShaderGameWorldLightingPass::Draw(mesh,meshIdx,renderBufferIndexBuffer,instanceCount);
}
#endif

void ShaderLightCone::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }
void ShaderLightCone::InitializeShaderResources()
{
	ShaderGameWorldLightingPass::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_DEPTH_MAP);
}
void ShaderLightCone::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo, prosper::CullModeFlags::None);
	pipelineInfo.ToggleDepthWrites(false);
}
