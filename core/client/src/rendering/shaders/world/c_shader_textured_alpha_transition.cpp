/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_textured_alpha_transition.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderTexturedAlphaTransition::VERTEX_BINDING_ALPHA) ShaderTexturedAlphaTransition::VERTEX_BINDING_ALPHA = {prosper::VertexInputRate::Vertex};
decltype(ShaderTexturedAlphaTransition::VERTEX_ATTRIBUTE_ALPHA) ShaderTexturedAlphaTransition::VERTEX_ATTRIBUTE_ALPHA = {VERTEX_BINDING_ALPHA, prosper::Format::R32G32_SFloat};
decltype(ShaderTexturedAlphaTransition::DESCRIPTOR_SET_MATERIAL) ShaderTexturedAlphaTransition::DESCRIPTOR_SET_MATERIAL = {&ShaderGameWorldLightingPass::DESCRIPTOR_SET_MATERIAL,
  {prosper::DescriptorSetInfo::Binding {// Diffuse Map 2
     prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit},
    prosper::DescriptorSetInfo::Binding {// Diffuse Map 3
      prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}}};
ShaderTexturedAlphaTransition::ShaderTexturedAlphaTransition(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "world/vs_textured_alpha_transition", "world/fs_textured_alpha_transition")
{
	// SetBaseShader<ShaderTextured3DBase>();
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderTexturedAlphaTransition::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto descSetGroup = ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(mat, DESCRIPTOR_SET_MATERIAL);
	if(descSetGroup == nullptr)
		return nullptr;
	auto &descSet = *descSetGroup->GetDescriptorSet();

	auto *diffuseMap2 = mat.GetTextureInfo("diffusemap2");
	if(diffuseMap2 != nullptr && diffuseMap2->texture != nullptr) {
		auto texture = std::static_pointer_cast<Texture>(diffuseMap2->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(), umath::to_integral(MaterialBinding::DiffuseMap2));
	}

	auto *diffuseMap3 = mat.GetTextureInfo("diffusemap3");
	if(diffuseMap3 != nullptr && diffuseMap3->texture != nullptr) {
		auto texture = std::static_pointer_cast<Texture>(diffuseMap3->texture);
		if(texture->HasValidVkTexture())
			descSet.SetBindingTexture(*texture->GetVkTexture(), umath::to_integral(MaterialBinding::DiffuseMap3));
	}
	return descSetGroup;
}
prosper::DescriptorSetInfo &ShaderTexturedAlphaTransition::GetMaterialDescriptorSetInfo() const { return DESCRIPTOR_SET_MATERIAL; }
void ShaderTexturedAlphaTransition::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo, pipelineIdx, 0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}
void ShaderTexturedAlphaTransition::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	AddVertexAttribute(pipelineInfo, VERTEX_ATTRIBUTE_ALPHA);
}
