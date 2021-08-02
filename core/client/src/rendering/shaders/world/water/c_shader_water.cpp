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
#include "pragma/entities/func/c_func_water.h"
#include "pragma/c_water_object.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderWater::DESCRIPTOR_SET_MATERIAL) ShaderWater::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::DescriptorSetInfo::Binding { // DuDv Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Normal Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderWater::DESCRIPTOR_SET_WATER) ShaderWater::DESCRIPTOR_SET_WATER = {
	{
		prosper::DescriptorSetInfo::Binding { // Reflection Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Refraction Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Refraction Depth
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Water settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Water fog
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderWater::ShaderWater(prosper::IPrContext &context,const std::string &identifier)
	: ShaderGameWorldLightingPass(context,identifier,"world/vs_water","world/fs_water")
{
	// SetBaseShader<ShaderTextured3DBase>();
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderWater::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();

	auto *dudvMap = mat.GetTextureInfo("dudvmap");
	if(dudvMap != nullptr && dudvMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(dudvMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(),umath::to_integral(MaterialBinding::DuDvMap));
	}

	auto *normalMap = mat.GetNormalMap();
	if(normalMap != nullptr && normalMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(normalMap->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(),umath::to_integral(MaterialBinding::NormalMap));
	}
	return descSetGroup;
}
bool ShaderWater::BeginDraw(const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin,RecordFlags recordFlags)
{
	if(ShaderGameWorldLightingPass::BeginDraw(cmdBuffer,clipPlane,drawOrigin,recordFlags) == false)
		return false;
	decltype(PushConstants::enableReflection) enableReflection = {m_bReflectionEnabled == true ? 1u : 0u};
	return RecordPushConstants(enableReflection,sizeof(ShaderGameWorldLightingPass::PushConstants) +offsetof(PushConstants,enableReflection));
}
void ShaderWater::SetReflectionEnabled(bool b) {m_bReflectionEnabled = b;}
bool ShaderWater::UpdateBindFogDensity()
{
	auto fogIntensity = 1.f;
	if(m_boundEntity.valid() == false || m_boundScene.expired() == true)
		return true;
	auto whWaterComponent = m_boundEntity->GetComponent<CWaterComponent>();
	if(whWaterComponent.expired())
		return true;
	auto &scene = *m_boundScene;
	auto &cam = scene.GetActiveCamera();
	if(cam.expired())
		return false;
	auto &pos = cam->GetEntity().GetPosition();
	if(whWaterComponent->IsPointBelowWaterPlane(pos) == true)
		fogIntensity = 0.f;
	return RecordPushConstants(fogIntensity,sizeof(ShaderGameWorldLightingPass::PushConstants) +offsetof(PushConstants,waterFogIntensity));
}

void ShaderWater::EndDraw()
{
	m_boundEntity = EntityHandle{};
	m_boundScene = decltype(m_boundScene){};
}

bool ShaderWater::BindSceneCamera(pragma::CSceneComponent &scene,const CRasterizationRendererComponent &renderer,bool bView)
{
	auto r = ShaderGameWorldLightingPass::BindSceneCamera(scene,renderer,bView);
	if(r == false)
		return false;
	auto &cam = scene.GetActiveCamera();
	if(cam.expired())
		return false;
	auto m = cam->GetProjectionMatrix() *cam->GetViewMatrix();
	m_boundScene = scene.GetHandle<pragma::CSceneComponent>();
	return UpdateBindFogDensity() &&
		RecordPushConstants(m,sizeof(ShaderGameWorldLightingPass::PushConstants) +offsetof(PushConstants,reflectionVp));
}

bool ShaderWater::BindEntity(CBaseEntity &ent)
{
	auto whWaterComponent = ent.GetComponent<CWaterComponent>();
	if(whWaterComponent.expired())
		return false;
	if(ShaderGameWorldLightingPass::BindEntity(ent) == false)
		return false;
	auto *descSetEffect = whWaterComponent->GetEffectDescriptorSet();
	if(descSetEffect == nullptr || whWaterComponent->IsWaterSceneValid() == false)
		return false;
	auto &waterScene = whWaterComponent->GetWaterScene();
	auto &sceneReflection = waterScene.sceneReflection;
//	auto &sceneRefraction = waterScene.sceneRefraction;
	//auto &rtReflection = sceneReflection->GetRenderTarget(); // prosper TODO
	//auto &rtRefraction = sceneRefraction->GetRenderTarget();

	//rtReflection->GetTexture()->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal); // prosper TODO

	//rtRefraction->GetTexture()->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal);
	//sceneRefraction->GetDepthTexture()->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal);
	m_boundEntity = ent.GetHandle();
	return UpdateBindFogDensity() && RecordBindDescriptorSet(*descSetEffect,DESCRIPTOR_SET_WATER.setIndex);
}

void ShaderWater::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::None);
	pipelineInfo.ToggleDepthWrites(true); // Water is not part of render pre-pass, but we need the depth for post-processing
}

void ShaderWater::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_WATER);
}

void ShaderWater::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderGameWorldLightingPass::PushConstants) +sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}

prosper::DescriptorSetInfo &ShaderWater::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
