#include "stdafx_client.h"
#include "pragma/rendering/shaders/image/c_shader_clear_color.hpp"
#include <prosper_context.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util_square_shape.hpp>
#include <vulkan/vulkan.hpp>
#include <wrappers/descriptor_set_group.h>

using namespace pragma;

decltype(ShaderClearColor::VERTEX_BINDING_VERTEX) ShaderClearColor::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderClearColor::VERTEX_ATTRIBUTE_POSITION) ShaderClearColor::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32_SFLOAT};
ShaderClearColor::ShaderClearColor(prosper::Context &context,const std::string &identifier)
	: ShaderGraphics(context,identifier,"screen/vs_screen","screen/fs_clear_color")
{}

ShaderClearColor::ShaderClearColor(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderGraphics(context,identifier,vsShader,fsShader,gsShader)
{}

void ShaderClearColor::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);
}

bool ShaderClearColor::Draw(const PushConstants &pushConstants)
{
	auto &dev = GetContext().GetDevice();
	if(
		RecordBindVertexBuffer(prosper::util::get_square_vertex_buffer(dev)->GetAnvilBuffer()) == false ||
		RecordPushConstants(pushConstants) == false ||
		RecordDraw(prosper::util::get_square_vertex_count()) == false
	)
		return false;
	return true;
}
