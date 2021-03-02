/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <prosper_util.hpp>

using namespace pragma;

decltype(ShaderPPFog::DESCRIPTOR_SET_TEXTURE) ShaderPPFog::DESCRIPTOR_SET_TEXTURE = {ShaderPPBase::DESCRIPTOR_SET_TEXTURE};
decltype(ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER) ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER = {
	{
		prosper::DescriptorSetInfo::Binding { // Depth Buffer
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderPPFog::DESCRIPTOR_SET_SCENE) ShaderPPFog::DESCRIPTOR_SET_SCENE = {&ShaderScene::DESCRIPTOR_SET_SCENE};
decltype(ShaderPPFog::DESCRIPTOR_SET_FOG) ShaderPPFog::DESCRIPTOR_SET_FOG = {
	{
		prosper::DescriptorSetInfo::Binding { // Fog
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderPPFog::ShaderPPFog(prosper::IPrContext &context,const std::string &identifier)
	: ShaderPPBase(context,identifier,"screen/fs_pp_fog")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderPPFog::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddDefaultVertexAttributes(pipelineInfo);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_DEPTH_BUFFER);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_FOG);
}

bool ShaderPPFog::Draw(prosper::IDescriptorSet &descSetTexture,prosper::IDescriptorSet &descSetDepth,prosper::IDescriptorSet &descSetCamera,prosper::IDescriptorSet &descSetFog)
{
	return RecordBindDescriptorSets({&descSetDepth,&descSetCamera,&descSetFog},DESCRIPTOR_SET_DEPTH_BUFFER.setIndex) &&
		ShaderPPBase::Draw(descSetTexture);
}
