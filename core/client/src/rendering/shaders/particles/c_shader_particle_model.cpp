/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_model.hpp"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderParticleModel::VERTEX_BINDING_PARTICLE) ShaderParticleModel::VERTEX_BINDING_PARTICLE = {prosper::VertexInputRate::Instance, sizeof(pragma::CParticleSystemComponent::ParticleData)};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_POSITION) ShaderParticleModel::VERTEX_ATTRIBUTE_POSITION = {pragma::ShaderParticle2DBase::VERTEX_ATTRIBUTE_POSITION, VERTEX_BINDING_PARTICLE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_RADIUS) ShaderParticleModel::VERTEX_ATTRIBUTE_RADIUS = {pragma::ShaderParticle2DBase::VERTEX_ATTRIBUTE_RADIUS, VERTEX_BINDING_PARTICLE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_PREVPOS) ShaderParticleModel::VERTEX_ATTRIBUTE_PREVPOS = {pragma::ShaderParticle2DBase::VERTEX_ATTRIBUTE_PREVPOS, VERTEX_BINDING_PARTICLE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_AGE) ShaderParticleModel::VERTEX_ATTRIBUTE_AGE = {pragma::ShaderParticle2DBase::VERTEX_ATTRIBUTE_AGE, VERTEX_BINDING_PARTICLE};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_COLOR) ShaderParticleModel::VERTEX_ATTRIBUTE_COLOR = {pragma::ShaderParticle2DBase::VERTEX_ATTRIBUTE_COLOR, VERTEX_BINDING_PARTICLE};

decltype(ShaderParticleModel::VERTEX_BINDING_ROTATION) ShaderParticleModel::VERTEX_BINDING_ROTATION = {prosper::VertexInputRate::Instance, sizeof(Quat)};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_ROTATION) ShaderParticleModel::VERTEX_ATTRIBUTE_ROTATION = {VERTEX_BINDING_ROTATION, prosper::Format::R32G32B32A32_SFloat};

decltype(ShaderParticleModel::VERTEX_BINDING_ANIMATION_START) ShaderParticleModel::VERTEX_BINDING_ANIMATION_START = {prosper::VertexInputRate::Instance};
decltype(ShaderParticleModel::VERTEX_ATTRIBUTE_ANIMATION_START) ShaderParticleModel::VERTEX_ATTRIBUTE_ANIMATION_START = {VERTEX_BINDING_ANIMATION_START, prosper::Format::R32_SFloat};

decltype(ShaderParticleModel::DESCRIPTOR_SET_ANIMATION) ShaderParticleModel::DESCRIPTOR_SET_ANIMATION = {{prosper::DescriptorSetInfo::Binding {prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit}}};
decltype(ShaderParticleModel::DESCRIPTOR_SET_BONE_MATRICES) ShaderParticleModel::DESCRIPTOR_SET_BONE_MATRICES = {{prosper::DescriptorSetInfo::Binding {// Bone Matrices
  prosper::DescriptorType::UniformBufferDynamic, prosper::ShaderStageFlags::VertexBit}}};
ShaderParticleModel::ShaderParticleModel(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "particles/model/vs_particle_model", "particles/model/fs_particle_model")
{
	SetPipelineCount(GetParticlePipelineCount());
	SetBaseShader<pragma::ShaderGameWorldLightingPass>();
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

	//pipelineInfo.ToggleDepthWrites(true);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_RADIUS);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_PREVPOS);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_AGE);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_COLOR);
	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_ROTATION);

	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_ANIMATION_START);
	ShaderParticleBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_ANIMATION);
	AddDescriptorSetGroup(pipelineInfo, pipelineIdx, DESCRIPTOR_SET_BONE_MATRICES);
}
void ShaderParticleModel::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}
bool ShaderParticleModel::RecordParticleSystem(prosper::ShaderBindState &bindState, pragma::CParticleSystemComponent &pSys) const
{
	auto &descSet = const_cast<ShaderParticleModel *>(this)->GetAnimationDescriptorSet(pSys);
	auto r = RecordBindDescriptorSet(bindState, descSet, DESCRIPTOR_SET_ANIMATION.setIndex);
	if(r == false)
		return r;
	PushConstants pushConstants {umath::to_integral(GetRenderFlags(pSys, ParticleRenderFlags::None)), // TODO: Use correct particle render flags
	  umath::to_integral(pSys.GetAlphaMode())};
	return RecordPushConstants(bindState, sizeof(pushConstants), &pushConstants, sizeof(ShaderGameWorldLightingPass::PushConstants));
}

bool ShaderParticleModel::RecordParticleBuffers(prosper::ShaderBindState &bindState, prosper::IBuffer &particleBuffer, prosper::IBuffer &rotBuffer, prosper::IBuffer &animStartBuffer)
{
	return RecordBindVertexBuffers(bindState, {&particleBuffer, &rotBuffer, &animStartBuffer}, VERTEX_BINDING_PARTICLE.GetBindingIndex());
}

bool ShaderParticleModel::Draw(CModelSubMesh &mesh, uint32_t numInstances, uint32_t firstInstance)
{
#if 0
	return ShaderGameWorldLightingPass::Draw(mesh,{},*CSceneComponent::GetEntityInstanceIndexBuffer()->GetZeroIndexBuffer(),[this,numInstances,firstInstance](CModelSubMesh &mesh) {
		return RecordDrawIndexed(mesh.GetTriangleVertexCount(),numInstances,0u,firstInstance);
	},true);
#endif
	return false;
}

bool ShaderParticleModel::RecordBeginDraw(prosper::ShaderBindState &bindState, const Vector4 &clipPlane, pragma::CParticleSystemComponent &pSys, const Vector4 &drawOrigin, ShaderScene::RecordFlags recordFlags) const
{
#if 0
	return ShaderGameWorldLightingPass::RecordBeginDraw(
		bindState,clipPlane,drawOrigin,
		//static_cast<ShaderGameWorldPipeline>(umath::to_integral(pipelineIdx) *umath::to_integral(AlphaMode::Count) +umath::to_integral(GetRenderAlphaMode(pSys))),
		recordFlags
	);
#endif
	return false;
}
