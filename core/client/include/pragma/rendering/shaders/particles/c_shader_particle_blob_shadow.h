/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_PARTICLE_BLOB_SHADOW_H__
#define __C_SHADER_PARTICLE_BLOB_SHADOW_H__
// prosper TODO
#if 0
#include "c_shader_particle_blob.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_shadow.h"
#include <cmaterial.h>

namespace Shader
{
	class DLLCLIENT ParticleBlobShadow
		: public ParticleShadowBase<ParticleBase>
	{
	public:
		enum class DLLCLIENT Location : std::underlying_type_t<ParticleBase::Location>
		{
			AdjacentBlobs = umath::to_integral(ParticleBase::Location::Xyzs) +1
		};
		enum class DLLCLIENT DescSet : std::underlying_type_t<ParticleBase::DescSet>
		{
			ParticleData = 0,
			LightData = ParticleData +1
		};
		enum class DLLCLIENT Binding : std::underlying_type_t<ParticleBase::Binding>
		{
			Camera = 0,
			ParticleData = 0,
			LightData = 0,

			AdjacentBlobs = umath::to_integral(ParticleBase::Binding::Xyzs) +1
		};
		static const uint32_t MAX_BLOB_NEIGHBORS = 8;
	protected:
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	public:
		ParticleBlobShadow();
		void Draw(CParticleSystem *particle,const Vulkan::Buffer &adjacentBlobBuffer,CLightBase *light,uint32_t layerId=0);
		bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descSetParticles,vk::DeviceSize offset);
		virtual uint32_t GetDescSet(ParticleBase::DescSet set) const override {return ParticleShadowBase<ParticleBase>::GetDescSet(set);}
	};
#if 0
	class DLLCLIENT ParticleBlobShadow
		: public ParticleShadowBase<ParticleBase>
	{
	public:
		/*enum class DLLCLIENT DescSet : uint32_t
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
		};*/
	public:
		ParticleBlobShadow();
		//void Draw(CParticleSystem *particle,const Vulkan::Buffer &vertexBuffer,const Vulkan::Buffer &indexBuffer,uint32_t indexCount,float radius,float curvature,CLightBase *light,uint32_t layerId=0);
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		//virtual void InitializeShaderPipelines(const Vulkan::Context &context) override {ParticleShadowBase<ParticleBlob>::InitializeShaderPipelines(context);}
		//virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		//virtual uint32_t GetDescSet(ParticleBase::DescSet set) const override {return ParticleShadowBase<ParticlePolyboard>::GetDescSet(set);}
	};
#endif
};
#endif
#endif