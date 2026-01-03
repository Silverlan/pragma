// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.module_toon;

import :engine;
import :entities.components;
import :rendering.shaders;

using namespace pragma::rendering::shader_graph;

//std::shared_ptr<prosper::IDescriptorSetGroup> ToonModule::g_defaultPbrDsg = {};
//size_t ToonModule::g_instanceCount = 0;
ToonModule::ToonModule(ShaderGraph &shader) : ShaderGraphModule {shader}
{
	/*m_pbrDescSetInfo = {
	  "TOON",
	  {prosper::DescriptorSetInfo::Binding {"IRRADIANCE_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
	    prosper::DescriptorSetInfo::Binding {"PREFILTER_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
	    prosper::DescriptorSetInfo::Binding {"BRDF_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
	};*/
}
ToonModule::~ToonModule()
{
	//if(--g_instanceCount == 0)
	//	g_defaultPbrDsg = nullptr;
}
void ToonModule::InitializeGfxPipelineDescriptorSets()
{
	/*m_shader.AddDescriptorSetGroup(m_pbrDescSetInfo);
	if(!g_defaultPbrDsg) {
		auto &context = pragma::get_cengine()->GetRenderContext();
		g_defaultPbrDsg = context.CreateDescriptorSetGroup(m_pbrDescSetInfo);
		auto &dummyTex = context.GetDummyTexture();
		auto &dummyCubemapTex = context.GetDummyCubemapTexture();
		auto &ds = *g_defaultPbrDsg->GetDescriptorSet(0);
		ds.SetBindingTexture(*dummyCubemapTex, pragma::math::to_integral(PBRBinding::IrradianceMap));
		ds.SetBindingTexture(*dummyCubemapTex, pragma::math::to_integral(PBRBinding::PrefilterMap));
		ds.SetBindingTexture(*dummyTex, pragma::math::to_integral(PBRBinding::BRDFMap));
	}*/
}
void ToonModule::RecordBindScene(ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	float iblStrength = 1.f;
	//auto *ds = GetReflectionProbeDescriptorSet(scene, iblStrength, inOutSceneFlags);

	//shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), m_pbrDescSetInfo.setIndex, *ds);
}
