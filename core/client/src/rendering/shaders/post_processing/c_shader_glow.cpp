/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_util.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;


decltype(ShaderGlow::DESCRIPTOR_SET_INSTANCE) ShaderGlow::DESCRIPTOR_SET_INSTANCE = {&ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderGlow::DESCRIPTOR_SET_SCENE) ShaderGlow::DESCRIPTOR_SET_SCENE = {&ShaderTextured3DBase::DESCRIPTOR_SET_SCENE};
decltype(ShaderGlow::DESCRIPTOR_SET_MATERIAL) ShaderGlow::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::DescriptorSetInfo::Binding { // Glow Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderGlow::RENDER_PASS_FORMAT) ShaderGlow::RENDER_PASS_FORMAT = prosper::Format::R8G8B8A8_UNorm;
ShaderGlow::ShaderGlow(prosper::IPrContext &context,const std::string &identifier)
	: ShaderTextured3DBase(context,identifier,"world/vs_glow","world/fs_glow")
{
	// SetBaseShader<ShaderTextured3DBase>();
}
prosper::DescriptorSetInfo &ShaderGlow::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
void ShaderGlow::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit);
}
void ShaderGlow::InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo,GetMaterialDescriptorSetInfo());
}
void ShaderGlow::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	pipelineInfo.ToggleDepthBias(true,-180.f /* constant factor */,-180.f /* clamp */,0.f /* slope factor */);
}
bool ShaderGlow::BeginDraw(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,Pipeline pipelineIdx)
{
	return ShaderSceneLit::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx));
}
uint32_t ShaderGlow::GetCameraDescriptorSetIndex() const {return DESCRIPTOR_SET_SCENE.setIndex;}
uint32_t ShaderGlow::GetInstanceDescriptorSetIndex() const {return DESCRIPTOR_SET_INSTANCE.setIndex;}
void ShaderGlow::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	CreateCachedRenderPass<ShaderGlow>({{{
		{
			RENDER_PASS_FORMAT,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::Clear,
			prosper::AttachmentStoreOp::Store,sampleCount,prosper::ImageLayout::ShaderReadOnlyOptimal
		},
		{
			RENDER_PASS_DEPTH_FORMAT,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AttachmentLoadOp::Load,
			prosper::AttachmentStoreOp::Store /* depth values have already been written by prepass */,sampleCount,prosper::ImageLayout::DepthStencilAttachmentOptimal
		}
	}}},outRenderPass,pipelineIdx);
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGlow::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto *glowMap = mat.GetGlowMap();
	if(glowMap == nullptr || glowMap->texture == nullptr)
		return nullptr;
	auto glowTexture = std::static_pointer_cast<Texture>(glowMap->texture);
	if(glowTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*glowTexture->GetVkTexture(),0u);
	return descSetGroup;
}
bool ShaderGlow::BindClipPlane(const Vector4 &clipPlane)
{
	// Clip plane currently not supported for glow shaders
	return true;
}
bool ShaderGlow::BindGlowMaterial(CMaterial &mat)
{
	auto *glowMap = mat.GetGlowMap();
	if(glowMap == nullptr || glowMap->texture == nullptr)
		return false;
	auto descSetGroup = mat.GetDescriptorSetGroup(*this);
	if(descSetGroup == nullptr)
		descSetGroup = InitializeMaterialDescriptorSet(mat); // Attempt to initialize on the fly
	if(descSetGroup == nullptr)
		return false;
	auto &data = mat.GetDataBlock();
	if(data != nullptr && data->GetBool("glow_alpha_only") == true)
	{
		auto texture = std::static_pointer_cast<Texture>(glowMap->texture);
		if(prosper::util::has_alpha(texture->GetVkTexture()->GetImage().GetFormat()) == false)
			return false;
	}
	auto scale = 1.f;
	if(data != nullptr)
		data->GetFloat("glow_scale",&scale);

	return RecordPushConstants(PushConstants{scale}) &&
		RecordBindDescriptorSet(*descSetGroup->GetDescriptorSet(),GetMaterialDescriptorSetIndex());
}

