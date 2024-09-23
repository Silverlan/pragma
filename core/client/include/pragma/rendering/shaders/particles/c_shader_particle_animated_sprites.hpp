/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PARTICLE_ANIMATED_SPRITES_HPP__
#define __C_SHADER_PARTICLE_ANIMATED_SPRITES_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

namespace pragma {
	class DLLCLIENT ShaderParticleAnimatedSprites : public ShaderParticle2DBase {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			float camBias;
		};
#pragma pack(pop)

		ShaderParticleAnimatedSprites(prosper::IPrContext &context, const std::string &identifier);
		virtual bool RecordDraw(prosper::ShaderBindState &bindState, pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const CParticleSystemComponent &ps, CParticleSystemComponent::OrientationType orientationType, ParticleRenderFlags renderFlags) override;
	  protected:
		virtual Vector3 DoCalcVertexPosition(const pragma::CParticleSystemComponent &ptc, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
