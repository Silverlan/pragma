/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_blob.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;
decltype(ShaderParticleBlob::VERTEX_BINDING_BLOB_NEIGHBORS) ShaderParticleBlob::VERTEX_BINDING_BLOB_NEIGHBORS = {prosper::VertexInputRate::Instance,MAX_BLOB_NEIGHBORS *sizeof(uint16_t)};
decltype(ShaderParticleBlob::VERTEX_ATTRIBUTE_BLOB_NEIGHBORS) ShaderParticleBlob::VERTEX_ATTRIBUTE_BLOB_NEIGHBORS = {VERTEX_BINDING_BLOB_NEIGHBORS,prosper::Format::R32G32B32A32_UInt};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_PARTICLE_DATA) ShaderParticleBlob::DESCRIPTOR_SET_PARTICLE_DATA = {
	{
		prosper::DescriptorSetInfo::Binding { // Particle data
			prosper::DescriptorType::StorageBufferDynamic,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_CUBEMAP) ShaderParticleBlob::DESCRIPTOR_SET_CUBEMAP = {
	{
		prosper::DescriptorSetInfo::Binding { // Cubemap
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderParticleBlob::ShaderParticleBlob(prosper::IPrContext &context,const std::string &identifier)
	: ShaderParticle2DBase(context,identifier,"particles/blob/vs_particle_blob","particles/blob/fs_particle_blob")
{
	SetBaseShader<ShaderParticle>();
}

void ShaderParticleBlob::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	
	RegisterDefaultGfxPipelineVertexAttributes(pipelineInfo);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BLOB_NEIGHBORS);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderParticle2DBase::PushConstants) +sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SHADOWS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_PARTICLE_DATA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CUBEMAP);
}
