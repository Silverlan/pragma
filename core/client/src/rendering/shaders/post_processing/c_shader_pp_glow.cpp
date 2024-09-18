/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_glow.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_util.hpp>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderPPGlow::DESCRIPTOR_SET_INSTANCE) ShaderPPGlow::DESCRIPTOR_SET_INSTANCE = {&ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderPPGlow::DESCRIPTOR_SET_SCENE) ShaderPPGlow::DESCRIPTOR_SET_SCENE = {&ShaderGameWorldLightingPass::DESCRIPTOR_SET_SCENE};
decltype(ShaderPPGlow::RENDER_PASS_FORMAT) ShaderPPGlow::RENDER_PASS_FORMAT = prosper::Format::R8G8B8A8_UNorm;
ShaderPPGlow::ShaderPPGlow(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "programs/scene/glow/glow", "programs/scene/glow/glow")
{
	m_shaderMaterialName = "glow";
	// SetBaseShader<ShaderTextured3DBase>();
}
void ShaderPPGlow::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit); }
void ShaderPPGlow::InitializeGfxPipelineDescriptorSets()
{
	AddDescriptorSetGroup(DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
}
void ShaderPPGlow::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	pipelineInfo.ToggleDepthBias(true, -180.f /* constant factor */, -180.f /* clamp */, 0.f /* slope factor */);
}
uint32_t ShaderPPGlow::GetCameraDescriptorSetIndex() const { return DESCRIPTOR_SET_SCENE.setIndex; }
uint32_t ShaderPPGlow::GetInstanceDescriptorSetIndex() const { return DESCRIPTOR_SET_INSTANCE.setIndex; }
void ShaderPPGlow::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	CreateCachedRenderPass<ShaderPPGlow>(
	  {{{{RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Clear, prosper::AttachmentStoreOp::Store, sampleCount, prosper::ImageLayout::ShaderReadOnlyOptimal},
	    {RENDER_PASS_DEPTH_FORMAT, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store /* depth values have already been written by prepass */, sampleCount, prosper::ImageLayout::DepthStencilAttachmentOptimal}}}},
	  outRenderPass, pipelineIdx);
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPPGlow::InitializeMaterialDescriptorSet(CMaterial &mat) { return ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(mat); }
bool ShaderPPGlow::RecordGlowMaterial(prosper::ShaderBindState &bindState, CMaterial &mat) const
{
	auto *glowMap = mat.GetGlowMap();
	if(glowMap == nullptr || glowMap->texture == nullptr)
		return false;
	auto descSetGroup = mat.GetDescriptorSetGroup(const_cast<ShaderPPGlow &>(*this));
	if(descSetGroup == nullptr)
		descSetGroup = const_cast<ShaderPPGlow *>(this)->InitializeMaterialDescriptorSet(mat); // Attempt to initialize on the fly
	if(descSetGroup == nullptr)
		return false;
	auto &data = mat.GetDataBlock();
	if(data != nullptr && data->GetBool("glow_alpha_only") == true) {
		auto texture = std::static_pointer_cast<Texture>(glowMap->texture);
		if(prosper::util::has_alpha(texture->GetVkTexture()->GetImage().GetFormat()) == false)
			return false;
	}
	auto scale = 1.f;
	if(data != nullptr)
		data->GetFloat("glow_scale", &scale);

	return false; // RecordPushConstants(bindState, PushConstants {scale}) && RecordBindDescriptorSet(bindState, *descSetGroup->GetDescriptorSet(), GetMaterialDescriptorSetIndex());
}
