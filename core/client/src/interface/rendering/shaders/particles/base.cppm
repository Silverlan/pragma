// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.shaders.particle_base;

export import :particle_system.enums;
export import :rendering.enums;
export import pragma.prosper;

export namespace pragma::ecs {
	class CParticleSystemComponent;
};
export namespace pragma {
	class DLLCLIENT ShaderParticleBase {
	  public:
		enum class RenderFlags : uint32_t { None = 0u, Animated = 1u, Unlit = Animated << 1, SoftParticles = Unlit << 1, TextureScrolling = SoftParticles << 1, AdditiveBlendByColor = TextureScrolling << 1, DepthPass = AdditiveBlendByColor << 1 };
		static uint32_t PIPELINE_COUNT;

		ShaderParticleBase() = default;
		RenderFlags GetRenderFlags(const ecs::CParticleSystemComponent &particle, pts::ParticleRenderFlags ptRenderFlags) const;
		static uint32_t GetDepthPipelineIndex();
	  protected:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_ANIMATION;
		virtual prosper::DescriptorSetInfo &GetAnimationDescriptorSetInfo() const = 0;
		prosper::IDescriptorSet &GetAnimationDescriptorSet(const ecs::CParticleSystemComponent &ps);
		uint32_t GetBasePipelineIndex(uint32_t pipelineIdx) const;
		rendering::ParticleAlphaMode GetAlphaMode(uint32_t pipelineIdx) const;
		rendering::ParticleAlphaMode GetRenderAlphaMode(const ecs::CParticleSystemComponent &particle) const;
		static uint32_t GetParticlePipelineCount();
		void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
	  private:
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dummyAnimDescSetGroup = nullptr;
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::ShaderParticleBase::RenderFlags)
};
