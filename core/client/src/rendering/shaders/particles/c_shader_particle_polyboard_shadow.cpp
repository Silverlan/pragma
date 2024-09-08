/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
// prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_polyboard_shadow.h"
#include "cmaterialmanager.h"
#include "pragma/model/c_side.h"
#include "pragma/rendering/uniformbinding.h"
#include "pragma/rendering/lighting/c_light_ranged.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <textureinfo.h>
#include <cmaterial.h>

using namespace Shader;

LINK_SHADER_TO_CLASS(ParticlePolyboardShadow,particlepolyboardshadow);

extern DLLCLIENT CGame *c_game;

ParticlePolyboardShadow::ParticlePolyboardShadow()
	: ParticleShadowBase<ParticlePolyboard>("particlepolyboardshadow","programs/particles/beam/particle_polyboard_shadow","programs/particles/beam/particle_polyboard_shadow","programs/particles/beam/particle_polyboard_shadow")
{
	m_bEnableLighting = false;
	SetUseDepth(true);
	SetUseAlpha(true);
}

void ParticlePolyboardShadow::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	ParticleShadowBase<ParticlePolyboard>::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.clear();
	pushConstants.push_back({prosper::ShaderStageFlags::GeometryBit,0,24});
	pushConstants.push_back({prosper::ShaderStageFlags::FragmentBit,24,1});
}

void ParticlePolyboardShadow::Draw(CParticleSystem *particle,const Vulkan::Buffer &vertexBuffer,const Vulkan::Buffer &indexBuffer,uint32_t indexCount,float radius,float curvature,CLightBase *light,uint32_t layerId)
{
	auto *mat = particle->GetMaterial();
	auto &descTexture = static_cast<CMaterial*>(mat)->GetDescriptorSet();
	if(!descTexture.IsValid())
		return;
	auto *shadow = light->GetShadowMap();
	auto *texShadow = shadow->GetDepthTexture();
	if(texShadow == nullptr)
		return;

	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();

	auto &mvp = light->GetTransformationMatrix(layerId);
	auto rot = particle->GetOrientation();
	//auto orType = particle->GetOrientationType();
	//auto w = context.GetWidth();
	//auto h = context.GetHeight();
	//auto texIntensity = 1.f;

	auto renderFlags = GetRenderFlags(particle);
	Vulkan::Std140LayoutBlockData fsPushConstants(1);
	fsPushConstants<<static_cast<int32_t>(renderFlags);

	drawCmd->PushConstants(layout,prosper::ShaderStageFlags::FragmentBit,24,static_cast<uint32_t>(fsPushConstants.GetCount()),fsPushConstants.GetData());

	auto *ranged = dynamic_cast<CLightRanged*>(light);
	auto &posLight = light->GetPosition();
	auto wShadow = (*texShadow)->GetWidth();
	auto hShadow = (*texShadow)->GetHeight();
	drawCmd->SetScissor(wShadow,hShadow);
	drawCmd->SetViewport(wShadow,hShadow);

	auto &cam = *c_game->GetRenderCamera();
	auto &v = cam.GetViewMatrix();
	auto vp = cam.GetProjectionMatrix() *v;
	Vulkan::Std140LayoutBlockData gsPushConstants(21);
	gsPushConstants<<mvp;
	gsPushConstants<<Vector4{posLight.x,posLight.y,posLight.z,(ranged != nullptr) ? static_cast<float>(ranged->GetDistance()) : 0.f};
	gsPushConstants<<radius<<curvature;

	drawCmd->PushConstants(layout,prosper::ShaderStageFlags::GeometryBit,static_cast<uint32_t>(gsPushConstants.GetCount()),gsPushConstants.GetData());

	auto bAnimated = ((renderFlags &RenderFlags::Animated) != RenderFlags::None) ? true : false;
	if(bAnimated == true)
	{
		auto *descSet = particle->GetAnimationDescriptorSet();
		drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::Animation),layout,*descSet);
	}
	drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::ParticleMap),layout,descTexture);

	drawCmd->BindVertexBuffer(vertexBuffer);
	drawCmd->BindIndexBuffer(indexBuffer,vk::IndexType::eUint16);
	drawCmd->DrawIndexed(indexCount);
}
#endif
