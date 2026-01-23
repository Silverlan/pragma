// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle_model_shadow;

import :engine;

// prosper TODO
#if 0

using namespace Shader;

LINK_SHADER_TO_CLASS(ParticleModelShadow,particlemodelshadow);


ParticleModelShadow::ParticleModelShadow()
	: Shadow("particlemodelshadow","programs/particles/model/particle_model_shadow","programs/particles/model/particle_model_shadow")
{}

bool ParticleModelShadow::BeginDrawTest(Vulkan::BufferObject *particleBuffer,Vulkan::BufferObject *rotBuffer,Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light,uint32_t w,uint32_t h)
{
	auto r = Shadow::BeginDrawTest(light,w,h);
	if(r == false)
		return r;
	cmdBuffer->BindVertexBuffer(pragma::math::to_integral(Binding::Xyzs),{particleBuffer,rotBuffer});
	return r;
}

void ParticleModelShadow::BindInstanceDescriptorSet(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSetObject *descSetInstance,const Mat4 &depthMvp)
{
	Shadow::BindInstanceDescriptorSet(cmdBuffer,descSetInstance);
	BindDepthMatrix(cmdBuffer,depthMvp);
}

void ParticleModelShadow::DrawTest(pragma::geometry::CModelSubMesh *mesh,uint32_t instanceCount)
{
	auto &context = pragma::get_cengine()->GetRenderContext();
	Shadow::DrawTest(context.GetDrawCmd(),mesh,[instanceCount](const Vulkan::CommandBufferObject *drawCmd,uint32_t triangleVertexCount) {
		drawCmd->DrawIndexed(0,triangleVertexCount,0,instanceCount);
	});
}

void ParticleModelShadow::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Shadow::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		pragma::math::to_integral(Binding::Xyzs),
		CParticleSystem::PARTICLE_DATA_SIZE,
		prosper::VertexInputRate::Instance
	});
	vertexBindingDescriptions.push_back({
		pragma::math::to_integral(Binding::Rotation),
		sizeof(Quat),
		prosper::VertexInputRate::Instance
	});

	vertexAttributeDescriptions.push_back({
		pragma::math::to_integral(Location::Xyzs),
		pragma::math::to_integral(Binding::Xyzs),
		prosper::Format::R32G32B32A32_SFloat,0
	});
	vertexAttributeDescriptions.push_back({
		pragma::math::to_integral(Location::Rotation),
		pragma::math::to_integral(Binding::Rotation),
		prosper::Format::R32G32B32A32_SFloat,0
	});
}
#endif
