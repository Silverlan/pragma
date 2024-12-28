/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_graph.hpp"
#include "pragma/rendering/shader_graph/modules/pbr.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include <prosper_command_buffer.hpp>

using namespace pragma::rendering::shader_graph;

extern DLLCLIENT CEngine *c_engine;
#pragma optimize("", off)
std::shared_ptr<prosper::IDescriptorSetGroup> PbrModule::g_defaultPbrDsg = {};
size_t PbrModule::g_instanceCount = 0;
PbrModule::PbrModule(ShaderGraph &shader) : pragma::rendering::ShaderGraphModule {shader}
{
	m_pbrDescSetInfo = {
	  "PBR",
	  {prosper::DescriptorSetInfo::Binding {"IRRADIANCE_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
	    prosper::DescriptorSetInfo::Binding {"PREFILTER_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
	    prosper::DescriptorSetInfo::Binding {"BRDF_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
	};
}
PbrModule::~PbrModule()
{
	if(--g_instanceCount == 0)
		g_defaultPbrDsg = nullptr;
}
void PbrModule::InitializeGfxPipelineDescriptorSets()
{
	m_shader.AddDescriptorSetGroup(m_pbrDescSetInfo);
	if(!g_defaultPbrDsg) {
		auto &context = c_engine->GetRenderContext();
		g_defaultPbrDsg = context.CreateDescriptorSetGroup(m_pbrDescSetInfo);
		auto &dummyTex = context.GetDummyTexture();
		auto &dummyCubemapTex = context.GetDummyCubemapTexture();
		auto &ds = *g_defaultPbrDsg->GetDescriptorSet(0);
		ds.SetBindingTexture(*dummyCubemapTex, umath::to_integral(PBRBinding::IrradianceMap));
		ds.SetBindingTexture(*dummyCubemapTex, umath::to_integral(PBRBinding::PrefilterMap));
		ds.SetBindingTexture(*dummyTex, umath::to_integral(PBRBinding::BRDFMap));
	}
}
void PbrModule::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	float iblStrength = 1.f;
	auto *ds = GetReflectionProbeDescriptorSet(scene, iblStrength, inOutSceneFlags);

	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), m_pbrDescSetInfo.setIndex, *ds);
}
prosper::IDescriptorSet *PbrModule::GetReflectionProbeDescriptorSet(const pragma::CSceneComponent &scene, float &outIblStrength, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	auto &hCam = scene.GetActiveCamera();
	assert(hCam.valid());
	auto *dsPbr = CReflectionProbeComponent::FindDescriptorSetForClosestProbe(scene, hCam->GetEntity().GetPosition(), outIblStrength);
	if(dsPbr == nullptr) // No reflection probe and therefore no IBL available. Fallback to non-IBL rendering.
	{
		dsPbr = &GetDefaultPbrDescriptorSet();
		inOutSceneFlags |= ShaderGameWorld::SceneFlags::NoIBL;
	}
	return dsPbr;
}
prosper::IDescriptorSet &PbrModule::GetDefaultPbrDescriptorSet() const { return *g_defaultPbrDsg->GetDescriptorSet(); }
