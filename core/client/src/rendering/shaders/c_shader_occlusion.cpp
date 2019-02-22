#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader_occlusion.h"
#include "pragma/rendering/uniformbinding.h"
#include <pragma/model/vertex.h>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace Shader;

LINK_SHADER_TO_CLASS(Occlusion,occlusion);

Occlusion::Occlusion()
	: Base3D("Occlusion","world/vs_occlusion","world/fs_occlusion")
{
	SetUseDepth(false);
	SetUseAlpha(false);
}

void Occlusion::InitializeAttachments(std::vector<vk::PipelineColorBlendAttachmentState> &attachments)
{
	attachments.clear(); // No attachments
}

bool Occlusion::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	if(Base3D::BeginDraw(cmdBuffer,shaderPipeline) == false)
		return false;
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto *pipeline = GetPipeline();
	auto &layout = pipeline->GetPipelineLayout();

	// Bind static buffers
	auto &bufStatic = *pipeline->GetDescriptorBuffer(umath::to_integral(DescSet::Camera));
	auto &descSetStatic = *bufStatic->GetDescriptorSet(context.GetFrameSwapIndex());
	drawCmd->BindDescriptorSet(umath::to_integral(DescSet::Camera),layout,descSetStatic);
	return true;
}

void Occlusion::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Base3D::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		umath::to_integral(Binding::Vertex),
		sizeof(Vertex),
		Anvil::VertexInputRate::VERTEX
	});

	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::Vertex),
		umath::to_integral(Binding::Vertex),
		Anvil::Format::R32G32B32_SFLOAT,0
	});
}

void Occlusion::InitializeShaderPipelines(const Vulkan::Context &context)
{
	Base3D::InitializeShaderPipelines(context);
}

void Occlusion::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base3D::InitializePipelineLayout(context,setLayouts,pushConstants);

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::VERTEX_BIT} // Camera
	}));
}

static Vulkan::QueryPool pool = nullptr;
void Occlusion::PrepareBatchRendering(unsigned int)
{
	auto &context = c_engine->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();

	if(pool == nullptr)
		pool = Vulkan::QueryPool::Create(context,vk::QueryType::eOcclusion,100);

	drawCmd->BeginQuery(pool,0);
	/*Use();
	BindIndexBuffer(indexBuffer);
	OpenGL::Disable(GL_CULL_FACE);
	OpenGL::SetColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glDepthMask(GL_FALSE);*/
}

void Occlusion::Render(unsigned int,unsigned int vertCount)
{

	//BindVertexBuffer(vertexBuffer);
	//OpenGL::DrawElements(GL_TRIANGLES,vertCount,GL_UNSIGNED_BYTE,(void*)0);
}

void Occlusion::EndBatchRendering()
{
	auto &context = c_engine->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	drawCmd->EndQuery(pool,0);
	/*OpenGL::DisableVertexAttribArray(SHADER_VERTEX_BUFFER_LOCATION);
	OpenGL::Enable(GL_CULL_FACE);
	OpenGL::SetColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glDepthMask(GL_TRUE);*/ // Vulkan TODO
}
#endif
