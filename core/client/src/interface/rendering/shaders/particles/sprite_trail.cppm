// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle_sprite_trail;

export import :rendering.shaders.particle_2d_base;

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
		virtual bool RecordDraw(prosper::ShaderBindState &bindState, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const ecs::CParticleSystemComponent &ps, pts::ParticleOrientationType orientationType, pts::ParticleRenderFlags renderFlags) override;
	  protected:
		virtual Vector3 DoCalcVertexPosition(const ecs::CParticleSystemComponent &ptc, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
