// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle_model_shadow;

// prosper TODO
#if 0
#include "definitions.hpp"
#include "c_shader_shadow.h"

namespace Shader
{
	class DLLCLIENT ParticleModelShadow
		: public Shadow
	{
	public:
		ParticleModelShadow();
		void BindInstanceDescriptorSet(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSetObject *descSetInstance,const Mat4 &depthMvp);
		bool BeginDrawTest(Vulkan::BufferObject *particleBuffer,Vulkan::BufferObject *rotBuffer,Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light,uint32_t w,uint32_t h);
		void DrawTest(pragma::geometry::CModelSubMesh *mesh,uint32_t instanceCount);
		enum class DLLCLIENT Location : uint32_t
		{
			Xyzs = pragma::math::to_integral(Shadow::Location::Vertex) +1,
			Rotation = Xyzs +1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Xyzs = pragma::math::to_integral(Shadow::Binding::BoneWeight) +1,
			Rotation = Xyzs +1
		};
	protected:
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
	};
};
#endif
