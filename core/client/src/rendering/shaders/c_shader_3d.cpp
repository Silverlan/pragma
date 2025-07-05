// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/shaders/c_shader_3d.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

Shader3DBase::Shader3DBase(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderGraphics(context, identifier, vsShader, fsShader, gsShader) {}
void Shader3DBase::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	pipelineInfo.ToggleDepthTest(true, prosper::CompareOp::LessOrEqual);
	pipelineInfo.ToggleDepthWrites(true);
}

void Shader3DBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { ShaderGraphics::InitializeRenderPass(outRenderPass, pipelineIdx); }
