/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/image/c_shader_clear_color.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_context.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>

using namespace pragma;

decltype(ShaderClearColor::VERTEX_BINDING_VERTEX) ShaderClearColor::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderClearColor::VERTEX_ATTRIBUTE_POSITION) ShaderClearColor::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32_SFloat};
ShaderClearColor::ShaderClearColor(prosper::IPrContext &context,const std::string &identifier)
	: ShaderGraphics(context,identifier,"screen/vs_screen","screen/fs_clear_color")
{}

ShaderClearColor::ShaderClearColor(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderGraphics(context,identifier,vsShader,fsShader,gsShader)
{}

void ShaderClearColor::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderClearColor::Draw(const PushConstants &pushConstants)
{
	if(
		RecordBindVertexBuffer(*GetContext().GetCommonBufferCache().GetSquareVertexBuffer()) == false ||
		RecordPushConstants(pushConstants) == false ||
		RecordDraw(prosper::CommonBufferCache::GetSquareVertexCount()) == false
	)
		return false;
	return true;
}
