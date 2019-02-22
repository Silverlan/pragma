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

 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "c_shader_particle_blob.h"
#include "cmaterialmanager.h"
#include "pragma/model/c_side.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/rendering/uniformbinding.h"
#include <textureinfo.h>
#include <cmaterial.h>

using namespace Shader;

LINK_SHADER_TO_CLASS(ParticleBlob,particleblob);

extern DLLCLIENT CGame *c_game;

ParticleBlob::ParticleBlob(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ParticleBase(identifier,vsShader,fsShader,gsShader)
{
	m_bEnableLighting = true;
	m_bUseParticleTexture = false;
	m_bUseSceneDepthTexture = false;
	SetUseDepth(true);
}

ParticleBlob::ParticleBlob()
	: ParticleBlob("particleblob","particles/blob/vs_particle_blob","particles/blob/fs_particle_blob")
{}

void ParticleBlob::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	ParticleBase::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		umath::to_integral(Binding::AdjacentBlobs),
		sizeof(uint16_t) *MAX_BLOB_NEIGHBORS,
		Anvil::VertexInputRate::INSTANCE
	});

	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::AdjacentBlobs),
		umath::to_integral(Binding::AdjacentBlobs),
		vk::Format::eR32G32B32A32Uint,0 // Each 4-byte uint actually packs two 2-byte uints
	});
}

void ParticleBlob::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	ParticleBase::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,16,7}); // Specular color, reflection refraction and debug mode
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::STORAGE_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Particle data
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Cubemap
	}));
}

uint32_t ParticleBlob::GetDescSet(DescSet set) const {return ParticleBase::GetDescSet(static_cast<ParticleBase::DescSet>(set));}

bool ParticleBlob::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descSetParticles,vk::DeviceSize offset,const Vector4 &specularColor,float reflectionIntensity,float refractionIndexRatio,DebugMode debugMode)
{
	if(ParticleBase::BeginDraw(cmdBuffer) == false)
		return false;
	auto &context = *m_context.get();
	auto &layout = GetPipeline()->GetPipelineLayout();

	// Bind particle buffer
	cmdBuffer->BindDescriptorSet(GetDescSet(DescSet::ParticleData),layout,descSetParticles,offset);

	if(reflectionIntensity > 0.f)
	{
		auto *mat = static_cast<ClientState*>(c_game->GetNetworkState())->LoadMaterial("skybox/dawn2"); // TODO: Use cubemap reflections
		if(mat != nullptr)
		{
			auto descSetMaterial = static_cast<CMaterial*>(mat)->GetDescriptorSet();
			if(descSetMaterial != nullptr)
				cmdBuffer->BindDescriptorSet(GetDescSet(DescSet::CubeMap),layout,descSetMaterial);
		}
	}

	Vulkan::Std140LayoutBlockData pushConstants(7);
	pushConstants<<specularColor<<reflectionIntensity<<refractionIndexRatio<<umath::to_integral(debugMode);
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,16,pushConstants.GetCount(),pushConstants.GetData());
	return true;
}

void ParticleBlob::Draw(Camera &cam,CParticleSystem *particle,Bool bloom,const Vulkan::Buffer &adjacentBlobBuffer)
{
	ParticleBase::Draw(cam,particle,bloom,[&adjacentBlobBuffer](const Vulkan::Context &context,const Vulkan::CommandBuffer &drawCmd) {
		drawCmd->BindVertexBuffer(umath::to_integral(Binding::AdjacentBlobs),adjacentBlobBuffer);
	});
}
#endif
