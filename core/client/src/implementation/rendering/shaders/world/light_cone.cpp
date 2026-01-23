// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_light_cone;

import :engine;
import :entities.components;
import :model;

using namespace pragma;

decltype(ShaderLightCone::DESCRIPTOR_SET_DEPTH_MAP) ShaderLightCone::DESCRIPTOR_SET_DEPTH_MAP = {
  "DEPTH_BUFFER",
  {prosper::DescriptorSetInfo::Binding {"MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderLightCone::ShaderLightCone(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "effects/vs_light_cone", "effects/fs_light_cone")
{
	// SetBaseShader<ShaderTextured3DBase>();
	math::set_flag(m_sceneFlags, SceneFlags::LightmapsEnabled, false);
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
bool ShaderLightCone::Draw(pragma::geometry::CModelSubMesh &mesh,const std::optional<pragma::rendering::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount)
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
