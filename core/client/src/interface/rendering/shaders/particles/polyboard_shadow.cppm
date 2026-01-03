// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle_polyboard_shadow;

// prosper TODO
#if 0
#include "definitions.hpp"

namespace Shader
{
	class DLLCLIENT ParticlePolyboardShadow
		: public ParticleShadowBase<ParticlePolyboard>
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			ParticleMap = 0,
			Animation = ParticleMap +1,
			Time = Animation +1
		};
		enum class DLLCLIENT Location : uint32_t
		{
			Vertex = 0,
			Color = 1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Vertex = 0,
			Color = 0
		};
	public:
		ParticlePolyboardShadow();
		void Draw(pragma::pts::CParticleSystem *particle,const Vulkan::Buffer &vertexBuffer,const Vulkan::Buffer &indexBuffer,uint32_t indexCount,float radius,float curvature,CLightBase *light,uint32_t layerId=0);
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override {ParticleShadowBase<ParticlePolyboard>::InitializeShaderPipelines(context);}
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual uint32_t GetDescSet(ParticleBase::DescSet set) const override {return ParticleShadowBase<ParticlePolyboard>::GetDescSet(set);}
	};
};
#endif
