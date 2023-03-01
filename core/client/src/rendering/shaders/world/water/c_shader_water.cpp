/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/liquid/c_liquid_surface_component.hpp"
#include "pragma/entities/components/c_surface_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/c_water_object.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderWater::DESCRIPTOR_SET_MATERIAL) ShaderWater::DESCRIPTOR_SET_MATERIAL = {{prosper::DescriptorSetInfo::Binding {// DuDv Map
                                                                                          prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Normal Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}}};
decltype(ShaderWater::DESCRIPTOR_SET_WATER) ShaderWater::DESCRIPTOR_SET_WATER = {{prosper::DescriptorSetInfo::Binding {// Reflection Map
                                                                                    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Refraction Map
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Refraction Depth
    prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Water settings
    prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit},
  prosper::DescriptorSetInfo::Binding {// Water fog
    prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit}}};
ShaderWater::ShaderWater(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "world/vs_textured", "world/fs_water")
{
	// SetBaseShader<ShaderTextured3DBase>();
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderWater::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this, descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();

	auto *dudvMap = mat.GetTextureInfo(Material::DUDV_MAP_IDENTIFIER);
	if(dudvMap != nullptr && dudvMap->texture != nullptr) {
		auto texture = std::static_pointer_cast<Texture>(dudvMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(), umath::to_integral(MaterialBinding::DuDvMap));
	}

	auto *normalMap = mat.GetNormalMap();
	if(normalMap != nullptr && normalMap->texture != nullptr) {
		auto texture = std::static_pointer_cast<Texture>(normalMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(), umath::to_integral(MaterialBinding::NormalMap));
	}
	return descSetGroup;
}
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
	auto posCam = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
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

void ShaderWater::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(pipelineInfo, pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_WATER);
}

void ShaderWater::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}

prosper::DescriptorSetInfo &ShaderWater::GetMaterialDescriptorSetInfo() const { return DESCRIPTOR_SET_MATERIAL; }
