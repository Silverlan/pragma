#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_model.hpp"
#include "pragma/model/c_modelmesh.h"
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

decltype(ShaderParticleModel::VERTEX_BINDING_PARTICLE) ShaderParticleModel::VERTEX_BINDING_PARTICLE = {Anvil::VertexInputRate::INSTANCE,sizeof(pragma::CParticleSystemComponent::ParticleData)};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_PARTICLE) ShaderParticleModel::VERTEX_ATTRIBUTE_PARTICLE = {pragma::ShaderParticle2DBase::VERTEX_ATTRIBUTE_PARTICLE,VERTEX_BINDING_PARTICLE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_COLOR) ShaderParticleModel::VERTEX_ATTRIBUTE_COLOR = {pragma::ShaderParticle2DBase::VERTEX_ATTRIBUTE_COLOR,VERTEX_BINDING_PARTICLE};

decltype(ShaderParticleModel::VERTEX_BINDING_ROTATION) ShaderParticleModel::VERTEX_BINDING_ROTATION = {Anvil::VertexInputRate::INSTANCE,sizeof(Quat)};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_ROTATION) ShaderParticleModel::VERTEX_ATTRIBUTE_ROTATION = {VERTEX_BINDING_ROTATION,Anvil::Format::R32G32B32A32_SFLOAT};

decltype(ShaderParticleModel::VERTEX_BINDING_ANIMATION_START) ShaderParticleModel::VERTEX_BINDING_ANIMATION_START = {Anvil::VertexInputRate::INSTANCE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_ANIMATION_START) ShaderParticleModel::VERTEX_ATTRIBUTE_ANIMATION_START = {VERTEX_BINDING_ANIMATION_START,Anvil::Format::R32_SFLOAT};

decltype(ShaderParticleModel::DESCRIPTOR_SET_ANIMATION) ShaderParticleModel::DESCRIPTOR_SET_ANIMATION = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderParticleModel::DESCRIPTOR_SET_BONE_MATRICES) ShaderParticleModel::DESCRIPTOR_SET_BONE_MATRICES = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Bone Matrices
			Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,
			Anvil::ShaderStageFlagBits::VERTEX_BIT
		}
	}
};
ShaderParticleModel::ShaderParticleModel(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase(context,identifier,"particles/model/vs_particle_model","particles/model/fs_particle_model")
{
	SetPipelineCount(GetParticlePipelineCount());
	SetBaseShader<ShaderTextured3D>();
}
prosper::Shader::DescriptorSetInfo &ShaderParticleModel::GetAnimationDescriptorSetInfo() const {return DESCRIPTOR_SET_ANIMATION;}
bool ShaderParticleModel::ShouldInitializePipeline(uint32_t pipelineIdx) {return ShaderTextured3DBase::ShouldInitializePipeline(GetBasePipelineIndex(pipelineIdx));}
void ShaderParticleModel::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(GetBasePipelineIndex(pipelineIdx));
	CreateCachedRenderPass<ShaderParticleModel>({{{
		{
			RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,
			Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
		},
		{
			RENDER_PASS_DEPTH_FORMAT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,
			Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	}}},outRenderPass,pipelineIdx);
}
void ShaderParticleModel::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	auto basePipelineIdx = GetBasePipelineIndex(pipelineIdx);
	ShaderTextured3DBase::InitializeGfxPipeline(pipelineInfo,basePipelineIdx);

	//pipelineInfo.toggle_depth_writes(true);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_PARTICLE);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_COLOR);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ROTATION);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_ANIMATION_START);
	ShaderParticleBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_ANIMATION);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_BONE_MATRICES);
}
void ShaderParticleModel::InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderTextured3DBase::PushConstants) +sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT);
}
bool ShaderParticleModel::BindParticleSystem(pragma::CParticleSystemComponent &pSys)
{
	auto &descSet = GetAnimationDescriptorSet(pSys);
	auto r = RecordBindDescriptorSet(descSet,DESCRIPTOR_SET_ANIMATION.setIndex);
	if(r == false)
		return r;
	PushConstants pushConstants {
		umath::to_integral(GetRenderFlags(pSys)),
		umath::to_integral(pSys.GetAlphaMode())
	};
	return RecordPushConstants(sizeof(pushConstants),&pushConstants,sizeof(ShaderTextured3DBase::PushConstants));
}

bool ShaderParticleModel::BindParticleBuffers(prosper::Buffer &particleBuffer,prosper::Buffer &rotBuffer,prosper::Buffer &animStartBuffer)
{
	return RecordBindVertexBuffers({&particleBuffer.GetAnvilBuffer(),&rotBuffer.GetAnvilBuffer(),&animStartBuffer.GetAnvilBuffer()},VERTEX_BINDING_PARTICLE.GetBindingIndex());
}

bool ShaderParticleModel::Draw(CModelSubMesh &mesh,uint32_t numInstances,uint32_t firstInstance)
{
	return ShaderTextured3DBase::Draw(mesh,[this,numInstances,firstInstance](CModelSubMesh &mesh) {
		return RecordDrawIndexed(mesh.GetTriangleVertexCount(),numInstances,0u,0,firstInstance);
	},true);
}

bool ShaderParticleModel::BeginDraw(
	const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,
	pragma::CParticleSystemComponent &pSys,Pipeline pipelineIdx,ShaderScene::RecordFlags recordFlags
)
{
	return ShaderTextured3DBase::BeginDraw(
		cmdBuffer,clipPlane,
		static_cast<Pipeline>(umath::to_integral(pipelineIdx) *umath::to_integral(AlphaMode::Count) +umath::to_integral(GetRenderAlphaMode(pSys))),
		recordFlags
	);
}

 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_model.hpp"
#include "pragma/model/c_modelmesh.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(ParticleModel,particlemodel);

extern DLLCLIENT CGame *c_game;

ParticleModel::ParticleModel()
	: Textured3D("particlemodel","particles/model/vs_particle_model","particles/model/fs_particle_model")
{
	//m_bEnableLighting = true;
	SetUseBloomAttachment(false);
}

void ParticleModel::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Textured3D::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		umath::to_integral(Binding::Xyzs),
		CParticleSystem::PARTICLE_DATA_SIZE,
		Anvil::VertexInputRate::INSTANCE
	});
	vertexBindingDescriptions.push_back({
		umath::to_integral(Binding::Rotation),
		sizeof(Quat),
		Anvil::VertexInputRate::INSTANCE
	});

	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::Xyzs),
		umath::to_integral(Binding::Xyzs),
		Anvil::Format::R32G32B32A32_SFLOAT,0
	});
	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::Color),
		umath::to_integral(Binding::Color),
		Anvil::Format::R16G16B16A16_UNORM,sizeof(Vector4)
	});
	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::Rotation),
		umath::to_integral(Binding::Rotation),
		Anvil::Format::R32G32B32A32_SFLOAT,0
	});
}

bool ParticleModel::BeginDraw(Vulkan::BufferObject *particleBuffer,Vulkan::BufferObject *rotBuffer,Vulkan::CommandBufferObject *cmdBuffer)
{
	auto r = Textured3D::BeginDraw(cmdBuffer);
	if(r == false)
		return r;
	cmdBuffer->BindVertexBuffer(umath::to_integral(Binding::Xyzs),{particleBuffer,rotBuffer});
	return r;
}

void ParticleModel::Draw(CModelSubMesh *mesh,uint32_t instanceCount)
{
	Textured3D::Draw(mesh,[instanceCount](const Vulkan::CommandBufferObject *drawCmd,std::size_t numTriangleVertices) {
		drawCmd->DrawIndexed(0,numTriangleVertices,0,instanceCount);
	});
}
#endif
