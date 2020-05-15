/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_copy_image.hpp>

using namespace pragma;

ShaderHDR::ShaderHDR(prosper::IPrContext &context,const std::string &identifier)
	: prosper::ShaderBaseImageProcessing(context,identifier,"screen/fs_hdr")
{
	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderHDR::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderHDR::Draw(prosper::IDescriptorSet &descSetTexture,float exposure)
{
	return RecordPushConstants(PushConstants{exposure}) &&
		prosper::ShaderBaseImageProcessing::Draw(descSetTexture);
}
