/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/debug/c_shader_debug_text.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

using namespace pragma;

decltype(ShaderDebugText::DESCRIPTOR_SET_TEXTURE) ShaderDebugText::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::DescriptorSetInfo::Binding {
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderDebugText::ShaderDebugText(prosper::IPrContext &context,const std::string &identifier)
	: ShaderDebug(context,identifier,"debug/vs_debug_text","debug/fs_debug_text")
{
	SetBaseShader<ShaderDebug>();
}

void ShaderDebugText::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderDebug::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
}

bool ShaderDebugText::Draw(prosper::IBuffer &vertexBuffer,uint32_t vertexCount,prosper::IDescriptorSet &descSetTexture,const Mat4 &mvp,const Vector4 &color)
{
	assert(vertexCount <= umath::to_integral(GameLimits::MaxMeshVertices));
	if(vertexCount > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw debug mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return false;
	}
	return RecordBindDescriptorSet(descSetTexture) && ShaderDebug::Draw(vertexBuffer,vertexCount,mvp,color);
}
