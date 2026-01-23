// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle_shadow;

// prosper TODO
#if 0

namespace Shader
{
	template<class TBase>
		class ParticleShadowBase
			: public TBase
	{
	protected:
		enum class DLLCLIENT DescSet : uint32_t
		{
			ParticleMap = 0,
			Animation = ParticleMap +1,
			Time = Animation +1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Time = 0
		};
	private:
		using TBase::GenerateDepthMapDescriptorSet;
		using TBase::GenerateParticleMapDescriptorSet;
		using TBase::GenerateAnimationDescriptorSet;
		using TBase::BindScene;
		using TBase::BindLights;
	protected:
		ParticleShadowBase(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		void InitializePushConstants(std::vector<Vulkan::PushConstantRange> &pushConstants) const;
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializeRenderPasses() override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual uint32_t GetDescSet(ParticleBase::DescSet set) const override;
	};
	class DLLCLIENT ParticleShadow
		: public ParticleShadowBase<ParticleBase>
	{
	public:
		ParticleShadow();
		void Draw(pragma::pts::CParticleSystem *particle,CLightBase *light,uint32_t layerId=0);
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override {ParticleShadowBase<ParticleBase>::InitializeShaderPipelines(context);}
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override {ParticleShadowBase<ParticleBase>::InitializePipelineLayout(context,setLayouts,pushConstants);}
		virtual uint32_t GetDescSet(ParticleBase::DescSet set) const override {return ParticleShadowBase<ParticleBase>::GetDescSet(set);}
	};
};

template<class TBase>
	Shader::ParticleShadowBase<TBase>::ParticleShadowBase(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
		: TBase(identifier,vsShader,fsShader,gsShader)
{
	SetUseAlpha(true);
	SetUseDepth(true);
	SetUseColorAttachments(false);
}

template<class TBase>
	void Shader::ParticleShadowBase<TBase>::InitializeShaderPipelines(const Vulkan::Context &context)
{
	Base::InitializeShaderPipelines(context);

	auto &timeBuffer = *pragma::get_cgame()->GetUniformBlockSwapBuffer(UniformBinding::Time);
	GetPipeline()->SetBuffer(pragma::math::to_integral(DescSet::Time),timeBuffer,pragma::math::to_integral(Binding::Time));
}

template<class TBase>
	void Shader::ParticleShadowBase<TBase>::InitializeRenderPasses() {m_renderPasses = {m_context->GenerateRenderPass(prosper::Format::D32_SFloat)};}

template<class TBase>
	void Shader::ParticleShadowBase<TBase>::InitializePushConstants(std::vector<Vulkan::PushConstantRange> &pushConstants) const
{
	pushConstants.push_back({prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,0,31});
}

template<class TBase>
	void Shader::ParticleShadowBase<TBase>::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);
	InitializePushConstants(pushConstants);

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{prosper::DescriptorType::CombinedImageSampler,prosper::ShaderStageFlags::FragmentBit} // Particle Map
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::FragmentBit} // Animation
	}));

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit} // Time
	}));
}
template<class TBase>
	uint32_t Shader::ParticleShadowBase<TBase>::GetDescSet(ParticleBase::DescSet set) const
{
	switch(set)
	{
		case ParticleBase::DescSet::ParticleMap:
			return pragma::math::to_integral(DescSet::ParticleMap);
		case ParticleBase::DescSet::Animation:
			return pragma::math::to_integral(DescSet::Animation);
		case ParticleBase::DescSet::Time:
			return pragma::math::to_integral(DescSet::Time);
	}
	return ParticleBase::GetDescSet(set);
}
#endif
