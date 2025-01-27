/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_graph.hpp"
#include "pragma/rendering/shader_graph/modules/toon.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include <prosper_command_buffer.hpp>

using namespace pragma::rendering::shader_graph;

extern DLLCLIENT CEngine *c_engine;
//std::shared_ptr<prosper::IDescriptorSetGroup> ToonModule::g_defaultPbrDsg = {};
//size_t ToonModule::g_instanceCount = 0;
ToonModule::ToonModule(ShaderGraph &shader) : pragma::rendering::ShaderGraphModule {shader}
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
		auto &context = c_engine->GetRenderContext();
		g_defaultPbrDsg = context.CreateDescriptorSetGroup(m_pbrDescSetInfo);
		auto &dummyTex = context.GetDummyTexture();
		auto &dummyCubemapTex = context.GetDummyCubemapTexture();
		auto &ds = *g_defaultPbrDsg->GetDescriptorSet(0);
		ds.SetBindingTexture(*dummyCubemapTex, umath::to_integral(PBRBinding::IrradianceMap));
		ds.SetBindingTexture(*dummyCubemapTex, umath::to_integral(PBRBinding::PrefilterMap));
		ds.SetBindingTexture(*dummyTex, umath::to_integral(PBRBinding::BRDFMap));
	}*/
}
void ToonModule::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	float iblStrength = 1.f;
	//auto *ds = GetReflectionProbeDescriptorSet(scene, iblStrength, inOutSceneFlags);

	//shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), m_pbrDescSetInfo.setIndex, *ds);
}
