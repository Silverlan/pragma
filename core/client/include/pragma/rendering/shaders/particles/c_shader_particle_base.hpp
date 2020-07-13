/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_PARTICLE_BASE_HPP__
#define __C_SHADER_PARTICLE_BASE_HPP__

#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/c_alpha_mode.hpp"

namespace pragma
{
	enum class ParticleRenderFlags : uint32_t;
	class CParticleSystemComponent;
	class DLLCLIENT ShaderParticleBase
	{
	public:
		enum class RenderFlags : uint32_t
		{
			None = 0u,
			Animated = 1u,
			Unlit = Animated<<1,
			SoftParticles = Unlit<<1,
			TextureScrolling = SoftParticles<<1,
			AdditiveBlendByColor = TextureScrolling<<1,
			DepthPass = AdditiveBlendByColor<<1
		};
		static uint32_t PIPELINE_COUNT;

		ShaderParticleBase()=default;
		RenderFlags GetRenderFlags(const CParticleSystemComponent &particle,ParticleRenderFlags ptRenderFlags) const;
		static uint32_t GetDepthPipelineIndex();
	protected:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_ANIMATION;
		virtual prosper::DescriptorSetInfo &GetAnimationDescriptorSetInfo() const=0;
		prosper::IDescriptorSet &GetAnimationDescriptorSet(const CParticleSystemComponent &ps);
		uint32_t GetBasePipelineIndex(uint32_t pipelineIdx) const;
		pragma::ParticleAlphaMode GetAlphaMode(uint32_t pipelineIdx) const;
		pragma::ParticleAlphaMode GetRenderAlphaMode(const CParticleSystemComponent &particle) const;
		static uint32_t GetParticlePipelineCount();
		void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
	private:
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dummyAnimDescSetGroup = nullptr;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderParticleBase::RenderFlags);

#endif
