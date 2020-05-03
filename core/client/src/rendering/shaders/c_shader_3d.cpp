/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_3d.hpp"

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

Shader3DBase::Shader3DBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderGraphics(context,identifier,vsShader,fsShader,gsShader)
{}
void Shader3DBase::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	pipelineInfo.toggle_depth_test(true,Anvil::CompareOp::LESS_OR_EQUAL);
	pipelineInfo.toggle_depth_writes(true);
}

void Shader3DBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeRenderPass(outRenderPass,pipelineIdx);
}
