// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

export module pragma.client.rendering.shaders:particle_sprite_trail;

export namespace pragma {
	class DLLCLIENT ShaderParticleSpriteTrail : public ShaderParticle2DBase {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			float minLength;
			float maxLength;
			float lengthFadeInTime;
			float animRate;
		};
#pragma pack(pop)

		ShaderParticleSpriteTrail(prosper::IPrContext &context, const std::string &identifier);
		virtual bool RecordDraw(prosper::ShaderBindState &bindState, pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const CParticleSystemComponent &ps, CParticleSystemComponent::OrientationType orientationType, ParticleRenderFlags renderFlags) override;
	  protected:
		virtual Vector3 DoCalcVertexPosition(const pragma::CParticleSystemComponent &ptc, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
