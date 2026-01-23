// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle_polyboard_shadow;

#if 0
using namespace Shader;

LINK_SHADER_TO_CLASS(ParticlePolyboardShadow,particlepolyboardshadow);


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

void ParticlePolyboardShadow::Draw(pragma::pts::CParticleSystem *particle,const Vulkan::Buffer &vertexBuffer,const Vulkan::Buffer &indexBuffer,uint32_t indexCount,float radius,float curvature,CLightBase *light,uint32_t layerId)
{
	auto *mat = particle->GetMaterial();
	auto &descTexture = static_cast<material::CMaterial*>(mat)->GetDescriptorSet();
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

	auto &cam = *pragma::get_cgame()->GetRenderCamera<pragma::CCameraComponent>();
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
