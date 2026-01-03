// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle_blob_shadow;

import :engine;
import :game;

// prosper TODO
#if 0

using namespace Shader;

LINK_SHADER_TO_CLASS(ParticleBlobShadow,particleblobshadow);


ParticleBlobShadow::ParticleBlobShadow()
	: ParticleShadowBase<ParticleBase>("ParticleBlobShadow","programs/particles/blob/particle_blob_shadow","programs/particles/blob/particle_blob_shadow")
{}

void ParticleBlobShadow::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	InitializeVertexDataDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		pragma::math::to_integral(Binding::AdjacentBlobs),
		sizeof(uint16_t) *MAX_BLOB_NEIGHBORS,
		prosper::VertexInputRate::Instance
	});

	vertexAttributeDescriptions.push_back({
		pragma::math::to_integral(Location::AdjacentBlobs),
		pragma::math::to_integral(Binding::AdjacentBlobs),
		vk::Format::eR32G32B32A32Uint,0 // Each 4-byte uint actually packs two 2-byte uints
	});
}

void ParticleBlobShadow::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	pushConstants.push_back({prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,0,28});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{prosper::DescriptorType::StorageBufferDynamic,prosper::ShaderStageFlags::FragmentBit} // Particle data
	}));

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit} // Light data
	}));
}

void ParticleBlobShadow::InitializeShaderPipelines(const Vulkan::Context &context)
{
	Base::InitializeShaderPipelines(context);
}

bool ParticleBlobShadow::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descSetParticles,vk::DeviceSize offset)
{
	if(Base::BeginDraw(cmdBuffer) == false)
		return false;
	auto &context = *m_context.get();
	auto &layout = GetPipeline()->GetPipelineLayout();

	auto &cam = *pragma::get_cgame()->GetRenderCamera<pragma::CCameraComponent>();
	auto &drawCmd = context.GetDrawCmd();

	// Bind particle buffer
	cmdBuffer->BindDescriptorSet(pragma::math::to_integral(DescSet::ParticleData),layout,descSetParticles,offset);
	return true;
}

void ParticleBlobShadow::Draw(pragma::pts::CParticleSystem *particle,const Vulkan::Buffer &adjacentBlobBuffer,CLightBase *light,uint32_t layerId)
{
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();

	auto rot = particle->GetOrientation();
	auto &mvp = light->GetTransformationMatrix(layerId);

	auto *shadow = light->GetShadowMap();
	auto *tex = shadow->GetDepthTexture();
	auto w = (*tex)->GetWidth();
	auto h = (*tex)->GetHeight();
	drawCmd->SetScissor(w,h);
	drawCmd->SetViewport(w,h);

	Vector3 right {};
	Vector3 up {};
	auto nearZ = 0.f;
	auto farZ = 0.f;
	GetParticleSystemInfo(nullptr,mvp,particle,nullptr,up,right,nearZ,farZ);

	auto &posLight = light->GetPosition();
	auto *ranged = dynamic_cast<CLightRanged*>(light);
	Vulkan::Std140LayoutBlockData instance(28);
	instance
		<<mvp
		<<right // Cam Right
		<<up // Cam Up
		<<Vector4{posLight.x,posLight.y,posLight.z,(ranged != nullptr) ? static_cast<float>(ranged->GetDistance()) : 0.f};
	drawCmd->PushConstants(layout,prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit,static_cast<uint32_t>(instance.GetCount()),instance.GetData());

	//drawCmd->BindDescriptorSet(pragma::math::to_integral(DescSet::LightData),layout,light->GetDescriptorSet());

	std::vector<Vulkan::BufferObject*> buffers = {
		particle->GetVertexBuffer(),
		*particle->GetParticleBuffer(),
		adjacentBlobBuffer
	};
	drawCmd->BindVertexBuffer(buffers);
	drawCmd->Draw(pragma::pts::CParticleSystem::VERTEX_COUNT,particle->GetRenderParticleCount());
}
#endif
