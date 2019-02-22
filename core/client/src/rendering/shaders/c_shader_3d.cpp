#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_3d.hpp"

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

Shader3DBase::Shader3DBase(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderGraphics(context,identifier,vsShader,fsShader,gsShader)
{}
void Shader3DBase::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	pipelineInfo.toggle_depth_test(true,Anvil::CompareOp::LESS_OR_EQUAL);
	pipelineInfo.toggle_depth_writes(true);
}

void Shader3DBase::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeRenderPass(outRenderPass,pipelineIdx);
}
