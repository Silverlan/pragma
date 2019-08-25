#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_blob.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;
decltype(ShaderParticleBlob::VERTEX_BINDING_VERTEX) ShaderParticleBlob::VERTEX_BINDING_BLOB_NEIGHBORS = {Anvil::VertexInputRate::INSTANCE,MAX_BLOB_NEIGHBORS *sizeof(uint16_t)};
decltype(ShaderParticleBlob::VERTEX_ATTRIBUTE_VERTEX) ShaderParticleBlob::VERTEX_ATTRIBUTE_BLOB_NEIGHBORS = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32A32_UINT};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_PARTICLE_DATA) ShaderParticleBlob::DESCRIPTOR_SET_PARTICLE_DATA = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Particle data
			Anvil::DescriptorType::STORAGE_BUFFER_DYNAMIC,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_CUBEMAP) ShaderParticleBlob::DESCRIPTOR_SET_CUBEMAP = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Cubemap
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderParticleBlob::ShaderParticleBlob(prosper::Context &context,const std::string &identifier)
	: ShaderParticle2DBase(context,identifier,"particles/blob/vs_particle_blob","particles/blob/fs_particle_blob")
{
	SetBaseShader<ShaderParticle>();
}

void ShaderParticleBlob::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	
	RegisterDefaultGfxPipelineVertexAttributes(pipelineInfo);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BLOB_NEIGHBORS);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderParticle2DBase::PushConstants) +sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CSM);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SHADOWS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_PARTICLE_DATA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CUBEMAP);
}
