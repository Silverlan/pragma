// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_water;

import :engine;
import :entities.components;

using namespace pragma;

decltype(ShaderWater::DESCRIPTOR_SET_MATERIAL) ShaderWater::DESCRIPTOR_SET_MATERIAL = {
  "MATERIAL",
  {prosper::DescriptorSetInfo::Binding {"SETTINGS", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::GeometryBit},
    prosper::DescriptorSetInfo::Binding {"DUDV_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"NORMAL_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderWater::DESCRIPTOR_SET_WATER) ShaderWater::DESCRIPTOR_SET_WATER = {
  "WATER",
  {prosper::DescriptorSetInfo::Binding {"REFLECTION_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"REFRACTION_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"REFRACTION_DEPTH", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"SETTINGS", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {"FOG", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderWater::ShaderWater(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "programs/scene/water", "programs/scene/water")
{
	m_shaderMaterialName = "water";
	// SetBaseShader<ShaderTextured3DBase>();
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderWater::InitializeMaterialDescriptorSet(material::CMaterial &mat) { return ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(mat); }
bool ShaderWater::RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const
{
	if(ShaderGameWorldLightingPass::RecordBindEntity(shaderProcessor, renderC, layout, entityInstanceDescriptorSetIndex) == false)
		return false;
	auto liquidSurfC = renderC.GetEntity().GetComponent<CLiquidSurfaceComponent>();
	if(liquidSurfC.expired() || liquidSurfC->IsWaterSceneValid() == false)
		return false;
	auto *ds = liquidSurfC->GetEffectDescriptorSet();
	if(!ds)
		return false;
	auto surfC = renderC.GetEntity().GetComponent<CSurfaceComponent>();
	if(surfC.expired())
		return false;
	// TODO: This is a bit messy, how can we do this a better way?
	auto &scene = shaderProcessor.GetCurrentScene();
	auto &cam = scene.GetActiveCamera();
	auto posCam = cam.valid() ? cam->GetEntity().GetPosition() : uvec::PRM_ORIGIN;
	PushConstants pushConstants {};
	if(surfC->IsPointBelowSurface(posCam)) {
		// Reflections aren't rendered if the camera is below the water surface, so we don't
		// want to sample the reflection map. Same goes for the fog.
		pushConstants.waterFogIntensity = 0.f;
		pushConstants.enableReflection = false;
	}
	else {
		pushConstants.waterFogIntensity = 1.f;
		pushConstants.enableReflection = true;
	}
	auto &cmd = shaderProcessor.GetCommandBuffer();
	return cmd.RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, layout, DESCRIPTOR_SET_WATER.setIndex, *ds) && cmd.RecordPushConstants(layout, prosper::ShaderStageFlags::FragmentBit, sizeof(ShaderGameWorldLightingPass::PushConstants), sizeof(PushConstants), &pushConstants);
}

GameShaderSpecializationConstantFlag ShaderWater::GetBaseSpecializationFlags() const { return ShaderGameWorldLightingPass::GetBaseSpecializationFlags() | GameShaderSpecializationConstantFlag::EnableTranslucencyBit; }

void ShaderWater::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
	// prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::None);
	pipelineInfo.ToggleDepthWrites(true); // Water is not part of render pre-pass, but we need the depth for post-processing
}

void ShaderWater::InitializeGfxPipelineDescriptorSets()
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets();
	AddDescriptorSetGroup(DESCRIPTOR_SET_WATER);
}

void ShaderWater::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }

prosper::DescriptorSetInfo &ShaderWater::GetMaterialDescriptorSetInfo() const { return DESCRIPTOR_SET_MATERIAL; }
