/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PARTICLE_HPP__
#define __C_SHADER_PARTICLE_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

namespace pragma {
	class CParticleSystemComponent;
	class DLLCLIENT ShaderParticle : public ShaderParticle2DBase {
	  public:
		ShaderParticle(prosper::IPrContext &context, const std::string &identifier);
	};

	class DLLCLIENT ShaderParticleRotational : public ShaderParticle2DBase {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_WORLD_ROTATION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_WORLD_ROTATION;

		enum class VertexAttribute : uint32_t { WorldRotation = umath::to_integral(ShaderParticle2DBase::VertexAttribute::Count) };

		bool RecordWorldRotationBuffer(prosper::ShaderBindState &bindState, prosper::IBuffer &buffer) const;
		ShaderParticleRotational(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void GetParticleSystemOrientationInfo(const Mat4 &matrix, const CParticleSystemComponent &particle, Vector3 &up, Vector3 &right, float &nearZ, float &farZ, const Material *material = nullptr, const pragma::CCameraComponent *cam = nullptr) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
