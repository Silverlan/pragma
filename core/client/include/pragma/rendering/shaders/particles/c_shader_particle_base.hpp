#ifndef __C_SHADER_PARTICLE_BASE_HPP__
#define __C_SHADER_PARTICLE_BASE_HPP__

#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/c_alpha_mode.hpp"

namespace pragma
{
	class CParticleSystemComponent;
	class DLLCLIENT ShaderParticleBase
	{
	public:
		enum class RenderFlags : uint32_t
		{
			None = 0u,
			Animated = 1u,
			Unlit = Animated<<1,
			BlackToAlpha = Unlit<<1,
			SoftParticles = BlackToAlpha<<1,
			TextureScrolling = SoftParticles<<1
		};

		ShaderParticleBase()=default;
		RenderFlags GetRenderFlags(const CParticleSystemComponent &particle) const;
	protected:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_ANIMATION;
		virtual prosper::Shader::DescriptorSetInfo &GetAnimationDescriptorSetInfo() const=0;
		Anvil::DescriptorSet &GetAnimationDescriptorSet(const CParticleSystemComponent &ps);
		uint32_t GetBasePipelineIndex(uint32_t pipelineIdx) const;
		pragma::AlphaMode GetAlphaMode(uint32_t pipelineIdx) const;
		pragma::AlphaMode GetRenderAlphaMode(const CParticleSystemComponent &particle) const;
		uint32_t GetParticlePipelineCount() const;
		void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
	private:
		std::shared_ptr<prosper::DescriptorSetGroup> m_dummyAnimDescSetGroup = nullptr;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderParticleBase::RenderFlags);

#endif
