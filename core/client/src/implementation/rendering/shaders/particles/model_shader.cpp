// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle_model;

import :engine;

using namespace pragma;

decltype(ShaderParticleModel::VERTEX_BINDING_PARTICLE) ShaderParticleModel::VERTEX_BINDING_PARTICLE = {prosper::VertexInputRate::Instance, sizeof(ecs::CParticleSystemComponent::ParticleData)};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_POSITION) ShaderParticleModel::VERTEX_ATTRIBUTE_POSITION = {ShaderParticle2DBase::VERTEX_ATTRIBUTE_POSITION, VERTEX_BINDING_PARTICLE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_RADIUS) ShaderParticleModel::VERTEX_ATTRIBUTE_RADIUS = {ShaderParticle2DBase::VERTEX_ATTRIBUTE_RADIUS, VERTEX_BINDING_PARTICLE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_PREVPOS) ShaderParticleModel::VERTEX_ATTRIBUTE_PREVPOS = {ShaderParticle2DBase::VERTEX_ATTRIBUTE_PREVPOS, VERTEX_BINDING_PARTICLE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_AGE) ShaderParticleModel::VERTEX_ATTRIBUTE_AGE = {ShaderParticle2DBase::VERTEX_ATTRIBUTE_AGE, VERTEX_BINDING_PARTICLE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_COLOR) ShaderParticleModel::VERTEX_ATTRIBUTE_COLOR = {ShaderParticle2DBase::VERTEX_ATTRIBUTE_COLOR, VERTEX_BINDING_PARTICLE};

decltype(ShaderParticleModel::VERTEX_BINDING_ROTATION) ShaderParticleModel::VERTEX_BINDING_ROTATION = {prosper::VertexInputRate::Instance, sizeof(Quat)};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_ROTATION) ShaderParticleModel::VERTEX_ATTRIBUTE_ROTATION = {VERTEX_BINDING_ROTATION, prosper::Format::R32G32B32A32_SFloat};

decltype(ShaderParticleModel::VERTEX_BINDING_ANIMATION_START) ShaderParticleModel::VERTEX_BINDING_ANIMATION_START = {prosper::VertexInputRate::Instance};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_ANIMATION_START) ShaderParticleModel::VERTEX_ATTRIBUTE_ANIMATION_START = {VERTEX_BINDING_ANIMATION_START, prosper::Format::R32_SFloat};

decltype(ShaderParticleModel::DESCRIPTOR_SET_ANIMATION) ShaderParticleModel::DESCRIPTOR_SET_ANIMATION = {
  "ANIMATION",
  {prosper::DescriptorSetInfo::Binding {"DATA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderParticleModel::DESCRIPTOR_SET_BONE_MATRICES) ShaderParticleModel::DESCRIPTOR_SET_BONE_MATRICES = {
  "BONES",
  {prosper::DescriptorSetInfo::Binding {"MATRIX_DATA", prosper::DescriptorType::UniformBufferDynamic, prosper::ShaderStageFlags::VertexBit}},
};
ShaderParticleModel::ShaderParticleModel(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "programs/particles/model/particle_model", "programs/particles/model/particle_model")
{
	SetPipelineCount(GetParticlePipelineCount());
	SetBaseShader<ShaderGameWorldLightingPass>();
}
prosper::DescriptorSetInfo &ShaderParticleModel::GetAnimationDescriptorSetInfo() const { return DESCRIPTOR_SET_ANIMATION; }
bool ShaderParticleModel::ShouldInitializePipeline(uint32_t pipelineIdx) { return ShaderGameWorldLightingPass::ShouldInitializePipeline(GetBasePipelineIndex(pipelineIdx)); }
void ShaderParticleModel::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(GetBasePipelineIndex(pipelineIdx));
	CreateCachedRenderPass<ShaderParticleModel>({{{{RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ColorAttachmentOptimal},
	                                              {RENDER_PASS_DEPTH_FORMAT, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::DepthStencilAttachmentOptimal}}}},
	  outRenderPass, pipelineIdx);
}
void ShaderParticleModel::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	auto basePipelineIdx = GetBasePipelineIndex(pipelineIdx);
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, basePipelineIdx);
	ShaderParticleBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
}
void ShaderParticleModel::InitializeShaderResources()
{
	ShaderGameWorldLightingPass::InitializeShaderResources();

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(VERTEX_ATTRIBUTE_RADIUS);
	AddVertexAttribute(VERTEX_ATTRIBUTE_PREVPOS);
	AddVertexAttribute(VERTEX_ATTRIBUTE_AGE);
	AddVertexAttribute(VERTEX_ATTRIBUTE_COLOR);
	AddVertexAttribute(VERTEX_ATTRIBUTE_ROTATION);

	AddVertexAttribute(VERTEX_ATTRIBUTE_ANIMATION_START);

	AddDescriptorSetGroup(DESCRIPTOR_SET_ANIMATION);
	AddDescriptorSetGroup(DESCRIPTOR_SET_BONE_MATRICES);
}
void ShaderParticleModel::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }
bool ShaderParticleModel::RecordParticleSystem(prosper::ShaderBindState &bindState, ecs::CParticleSystemComponent &pSys) const
{
	auto &descSet = const_cast<ShaderParticleModel *>(this)->GetAnimationDescriptorSet(pSys);
	auto r = RecordBindDescriptorSet(bindState, descSet, DESCRIPTOR_SET_ANIMATION.setIndex);
	if(r == false)
		return r;
	PushConstants pushConstants {math::to_integral(GetRenderFlags(pSys, pts::ParticleRenderFlags::None)), // TODO: Use correct particle render flags
	  math::to_integral(pSys.GetAlphaMode())};
	return RecordPushConstants(bindState, sizeof(pushConstants), &pushConstants, sizeof(ShaderGameWorldLightingPass::PushConstants));
}

bool ShaderParticleModel::RecordParticleBuffers(prosper::ShaderBindState &bindState, prosper::IBuffer &particleBuffer, prosper::IBuffer &rotBuffer, prosper::IBuffer &animStartBuffer)
{
	return RecordBindVertexBuffers(bindState, {&particleBuffer, &rotBuffer, &animStartBuffer}, VERTEX_BINDING_PARTICLE.GetBindingIndex());
}

bool ShaderParticleModel::Draw(geometry::CModelSubMesh &mesh, uint32_t numInstances, uint32_t firstInstance)
{
#if 0
	return ShaderGameWorldLightingPass::Draw(mesh,{},*CSceneComponent::GetEntityInstanceIndexBuffer()->GetZeroIndexBuffer(),[this,numInstances,firstInstance](pragma::geometry::CModelSubMesh &mesh) {
		return RecordDrawIndexed(mesh.GetTriangleVertexCount(),numInstances,0u,firstInstance);
	},true);
#endif
	return false;
}

bool ShaderParticleModel::RecordBeginDraw(prosper::ShaderBindState &bindState, const Vector4 &clipPlane, ecs::CParticleSystemComponent &pSys, const Vector4 &drawOrigin, RecordFlags recordFlags) const
{
#if 0
	return ShaderGameWorldLightingPass::RecordBeginDraw(
		bindState,clipPlane,drawOrigin,
		//static_cast<ShaderGameWorldPipeline>(pragma::math::to_integral(pipelineIdx) *pragma::math::to_integral(AlphaMode::Count) +pragma::math::to_integral(GetRenderAlphaMode(pSys))),
		recordFlags
	);
#endif
	return false;
}
