/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PARTICLE_POLYBOARD_SHADOW_H__
#define __C_SHADER_PARTICLE_POLYBOARD_SHADOW_H__
// prosper TODO
#if 0
#include "c_shader_particle_polyboard.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_shadow.h"
#include <cmaterial.h>

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
		void Draw(CParticleSystem *particle,const Vulkan::Buffer &vertexBuffer,const Vulkan::Buffer &indexBuffer,uint32_t indexCount,float radius,float curvature,CLightBase *light,uint32_t layerId=0);
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override {ParticleShadowBase<ParticlePolyboard>::InitializeShaderPipelines(context);}
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual uint32_t GetDescSet(ParticleBase::DescSet set) const override {return ParticleShadowBase<ParticlePolyboard>::GetDescSet(set);}
	};
};
#endif
#endif
