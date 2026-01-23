// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle_sprite_trail;

import :engine;
import :particle_system;

using namespace pragma;

ShaderParticleSpriteTrail::ShaderParticleSpriteTrail(prosper::IPrContext &context, const std::string &identifier) : ShaderParticle2DBase(context, identifier, "programs/pfm/particles/particle_sprite_trail", "programs/particles/particle") {}

Vector3 ShaderParticleSpriteTrail::DoCalcVertexPosition(const ecs::CParticleSystemComponent &ptc, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ) const
{
	// Note: This has to match the calculations performed in the vertex shader
	auto &renderers = ptc.GetRenderers();
	if(renderers.empty())
		return camPos;
	auto &renderer = renderers.front();
	if(typeid(*renderer) != typeid(pts::CParticleRendererSpriteTrail))
		return camPos;
	auto &rendererSt = *static_cast<pts::CParticleRendererSpriteTrail *>(renderer.get());
	auto *pt = const_cast<ecs::CParticleSystemComponent &>(ptc).GetParticle(ptIdx);
	auto &ptWorldPos = pt->GetPosition();
	auto &ptPrevWorldPos = pt->GetPrevPos();
	auto dtPosWs = ptPrevWorldPos - ptWorldPos;
	auto l = uvec::length(dtPosWs);
	uvec::normalize(&dtPosWs);

	auto dt = 1.0 / (1.0 / 100.0); // 100 is arbitrary, try using actual delta time!
	auto age = pt->GetTimeAlive();
	auto lengthFadeInTime = rendererSt.GetLengthFadeInTime();
	auto lengthScale = (age >= lengthFadeInTime) and 1.0 or (age / lengthFadeInTime);
	auto ptLen = pt->GetLength();
	l = lengthScale * l * ptLen; // *dt
	l = logf(l + 2) * 12;        //GCC stl: It still smells.
	if(l <= 0.0)
		return camPos;
	l = math::clamp(l, rendererSt.GetMinLength(), rendererSt.GetMaxLength());

	auto rad = math::min(pt->GetRadius(), l);
	dtPosWs = dtPosWs * l;

	auto dirToBeam = ptWorldPos - camPos;
	auto tangentY = uvec::cross(dirToBeam, dtPosWs);
	uvec::normalize(&tangentY);

	if(localVertIdx == 0)
		return ptWorldPos - tangentY * rad * 0.5f;
	if(localVertIdx == 1)
		return ptWorldPos + tangentY * rad * 0.5f;
	if(localVertIdx == 2)
		return (ptWorldPos + tangentY * rad * 0.5f) + dtPosWs;
	return (ptWorldPos - tangentY * rad * 0.5f) + dtPosWs;
}

void ShaderParticleSpriteTrail::InitializeShaderResources()
{
	ShaderSceneLit::InitializeShaderResources();

	RegisterDefaultGfxPipelineVertexAttributes();
	AttachPushConstantRange(0u, sizeof(ShaderParticle2DBase::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
	RegisterDefaultGfxPipelineDescriptorSetGroups();
}
void ShaderParticleSpriteTrail::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	auto basePipelineIdx = GetBasePipelineIndex(pipelineIdx);
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo, basePipelineIdx);

	pipelineInfo.ToggleDepthWrites(pipelineIdx == GetDepthPipelineIndex()); // Last pipeline is depth pipeline

	ShaderParticleBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
}

bool ShaderParticleSpriteTrail::RecordDraw(prosper::ShaderBindState &bindState, CSceneComponent &scene, const CRasterizationRendererComponent &r, const ecs::CParticleSystemComponent &ps, pts::ParticleOrientationType orientationType, pts::ParticleRenderFlags renderFlags)
{
	auto &renderers = ps.GetRenderers();
	if(renderers.empty())
		return false;
	auto &renderer = renderers.front();
	if(typeid(*renderer) != typeid(pts::CParticleRendererSpriteTrail))
		return false;
	auto &rendererSt = *static_cast<pts::CParticleRendererSpriteTrail *>(renderer.get());

	PushConstants pushConstants {};
	pushConstants.minLength = rendererSt.GetMinLength();
	pushConstants.maxLength = rendererSt.GetMaxLength();
	pushConstants.lengthFadeInTime = rendererSt.GetLengthFadeInTime();
	pushConstants.animRate = rendererSt.GetAnimationRate();
	return RecordPushConstants(bindState, pushConstants) && ShaderParticle2DBase::RecordDraw(bindState, scene, r, ps, orientationType, renderFlags);
}
