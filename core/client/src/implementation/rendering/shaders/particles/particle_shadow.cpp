// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle_shadow;

// prosper TODO
#if 0

using namespace Shader;

LINK_SHADER_TO_CLASS(ParticleShadow,particleshadow);


ParticleShadow::ParticleShadow()
	: ParticleShadowBase<ParticleBase>("ParticleShadow","programs/particles/particle_shadow","programs/particles/particle_shadow")
{}

void ParticleShadow::Draw(pragma::pts::CParticleSystem *particle,CLightBase *light,uint32_t layerId)
{
	auto *mat = particle->GetMaterial();
	auto &descTexture = static_cast<material::CMaterial*>(mat)->GetDescriptorSet();
	if(!descTexture.IsValid())
		return;
	auto *shadow = light->GetShadowMap();
	auto *tex = shadow->GetDepthTexture();
	if(tex == nullptr)
		return;

	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();

	auto rot = particle->GetOrientation();
	auto orType = particle->GetOrientationType();
	auto &mvp = light->GetTransformationMatrix(layerId);

	auto w = (*tex)->GetWidth();
	auto h = (*tex)->GetHeight();
	drawCmd->SetScissor(w,h);
	drawCmd->SetViewport(w,h);

	Vector3 right {};
	Vector3 up {};
	auto nearZ = 0.f;
	auto farZ = 0.f;
	GetParticleSystemInfo(nullptr,mvp,particle,mat,up,right,nearZ,farZ);

	auto &posLight = light->GetPosition();
	auto renderFlags = GetRenderFlags(particle);
	auto *ranged = dynamic_cast<CLightRanged*>(light);
	Vulkan::Std140LayoutBlockData instance(31);
	instance
		<<mvp
		<<right // Cam Right
		<<static_cast<int32_t>(orType)
		<<up // Cam Up
		<<Vector4{posLight.x,posLight.y,posLight.z,(ranged != nullptr ) ? static_cast<float>(ranged->GetDistance()) : 0.f}
		<<static_cast<int32_t>(renderFlags)
		<<nearZ<<farZ;
	drawCmd->PushConstants(layout,prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit,static_cast<uint32_t>(instance.GetCount()),instance.GetData());

	auto bAnimated = ((renderFlags &RenderFlags::Animated) != RenderFlags::None) ? true : false;
	if(bAnimated == true)
	{
		auto *descSet = particle->GetAnimationDescriptorSet();
		drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::Animation),layout,*descSet);
	}
	//auto &scene = pragma::get_cgame()->GetRenderScene();
	drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::ParticleMap),layout,descTexture);

	std::vector<Vulkan::BufferObject*> buffers = {
		particle->GetVertexBuffer(),
		*particle->GetParticleBuffer(),
		(bAnimated == true) ? *particle->GetAnimationStartBuffer() : pragma::get_cgame()->GetDummyVertexBuffer()
	};
	drawCmd->BindVertexBuffer(buffers);
	drawCmd->Draw(pragma::pts::CParticleSystem::VERTEX_COUNT,particle->GetRenderParticleCount());
}
#endif
