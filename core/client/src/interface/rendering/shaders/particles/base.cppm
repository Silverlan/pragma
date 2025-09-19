// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client.rendering.shaders:particle_base;

export namespace pragma {
	class DLLCLIENT ShaderParticleBase {
	  public:
		enum class RenderFlags : uint32_t { None = 0u, Animated = 1u, Unlit = Animated << 1, SoftParticles = Unlit << 1, TextureScrolling = SoftParticles << 1, AdditiveBlendByColor = TextureScrolling << 1, DepthPass = AdditiveBlendByColor << 1 };
		static uint32_t PIPELINE_COUNT;

		ShaderParticleBase() = default;
		RenderFlags GetRenderFlags(const CParticleSystemComponent &particle, ParticleRenderFlags ptRenderFlags) const;
		static uint32_t GetDepthPipelineIndex();
	  protected:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_ANIMATION;
		virtual prosper::DescriptorSetInfo &GetAnimationDescriptorSetInfo() const = 0;
		prosper::IDescriptorSet &GetAnimationDescriptorSet(const CParticleSystemComponent &ps);
		uint32_t GetBasePipelineIndex(uint32_t pipelineIdx) const;
		pragma::ParticleAlphaMode GetAlphaMode(uint32_t pipelineIdx) const;
		pragma::ParticleAlphaMode GetRenderAlphaMode(const CParticleSystemComponent &particle) const;
		static uint32_t GetParticlePipelineCount();
		void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
	  private:
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dummyAnimDescSetGroup = nullptr;
	};
};
export {
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderParticleBase::RenderFlags);
};
