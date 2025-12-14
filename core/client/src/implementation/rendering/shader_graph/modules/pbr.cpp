// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.shader_graph.module_pbr;

import :engine;
import :entities.components;
import :rendering.shaders;

using namespace pragma::rendering::shader_graph;

#pragma optimize("", off)
std::shared_ptr<prosper::IDescriptorSetGroup> PbrModule::g_defaultPbrDsg = {};
size_t PbrModule::g_instanceCount = 0;
PbrModule::PbrModule(ShaderGraph &shader) : ShaderGraphModule {shader}
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
		auto &context = get_cengine()->GetRenderContext();
		g_defaultPbrDsg = context.CreateDescriptorSetGroup(m_pbrDescSetInfo);
		auto &dummyTex = context.GetDummyTexture();
		auto &dummyCubemapTex = context.GetDummyCubemapTexture();
		auto &ds = *g_defaultPbrDsg->GetDescriptorSet(0);
		ds.SetBindingTexture(*dummyCubemapTex, math::to_integral(PBRBinding::IrradianceMap));
		ds.SetBindingTexture(*dummyCubemapTex, math::to_integral(PBRBinding::PrefilterMap));
		ds.SetBindingTexture(*dummyTex, math::to_integral(PBRBinding::BRDFMap));
	}
}
void PbrModule::RecordBindScene(ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	float iblStrength = 1.f;
	auto *ds = GetReflectionProbeDescriptorSet(scene, iblStrength, inOutSceneFlags);

	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), m_pbrDescSetInfo.setIndex, *ds);
}
prosper::IDescriptorSet *PbrModule::GetReflectionProbeDescriptorSet(const CSceneComponent &scene, float &outIblStrength, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
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
