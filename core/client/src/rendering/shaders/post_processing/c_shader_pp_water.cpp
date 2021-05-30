/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_water.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderPPWater::DESCRIPTOR_SET_TEXTURE) ShaderPPWater::DESCRIPTOR_SET_TEXTURE = {&ShaderPPFog::DESCRIPTOR_SET_TEXTURE};
decltype(ShaderPPWater::DESCRIPTOR_SET_DEPTH_BUFFER) ShaderPPWater::DESCRIPTOR_SET_DEPTH_BUFFER = {&ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER};
decltype(ShaderPPWater::DESCRIPTOR_SET_REFRACTION_MAP) ShaderPPWater::DESCRIPTOR_SET_REFRACTION_MAP = {
	{
		prosper::DescriptorSetInfo::Binding { // Refraction Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderPPWater::DESCRIPTOR_SET_SCENE) ShaderPPWater::DESCRIPTOR_SET_SCENE = {&ShaderScene::DESCRIPTOR_SET_SCENE};
decltype(ShaderPPWater::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderPPWater::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderPPWater::DESCRIPTOR_SET_FOG) ShaderPPWater::DESCRIPTOR_SET_FOG = {&ShaderPPFog::DESCRIPTOR_SET_FOG};
ShaderPPWater::ShaderPPWater(prosper::IPrContext &context,const std::string &identifier)
	: ShaderPPBase(context,identifier,"screen/fs_pp_water")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderPPWater::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_DEPTH_BUFFER);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_REFRACTION_MAP);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_FOG);
	AddDefaultVertexAttributes(pipelineInfo);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit);
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPPWater::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto *dudvMap = mat.GetTextureInfo("dudvmap");
	if(dudvMap == nullptr || dudvMap->texture == nullptr)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_REFRACTION_MAP);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	auto texture = std::static_pointer_cast<Texture>(dudvMap->texture);
	if(texture->HasValidVkTexture())
		descSet.SetBindingTexture(*texture->GetVkTexture(),0u);
	return descSetGroup;
}

bool ShaderPPWater::BindRefractionMaterial(CMaterial &mat)
{
	auto descSetGroup = mat.GetDescriptorSetGroup(*this);
	if(descSetGroup == nullptr)
		descSetGroup = InitializeMaterialDescriptorSet(mat); // Attempt to initialize on the fly
	if(descSetGroup == nullptr)
		return false;
	return RecordBindDescriptorSet(*descSetGroup->GetDescriptorSet(),DESCRIPTOR_SET_REFRACTION_MAP.setIndex);
}

bool ShaderPPWater::Draw(
	prosper::IDescriptorSet &descSetTexture,prosper::IDescriptorSet &descSetDepth,prosper::IDescriptorSet &descSetCamera,
	prosper::IDescriptorSet &descSetTime,prosper::IDescriptorSet &descSetFog,const Vector4 &clipPlane
)
{
	return RecordBindDescriptorSet(descSetDepth,DESCRIPTOR_SET_DEPTH_BUFFER.setIndex) &&
		RecordBindDescriptorSets({&descSetCamera,&descSetTime,&descSetFog},DESCRIPTOR_SET_SCENE.setIndex) &&
		RecordPushConstants(clipPlane) &&
		ShaderPPBase::Draw(descSetTexture);
}
