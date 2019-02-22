#include "stdafx_client.h"
#include "pragma/rendering/shaders/debug/c_shader_debug_text.hpp"

using namespace pragma;

decltype(ShaderDebugText::DESCRIPTOR_SET_TEXTURE) ShaderDebugText::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderDebugText::ShaderDebugText(prosper::Context &context,const std::string &identifier)
	: ShaderDebug(context,identifier,"debug/vs_debug_text","debug/fs_debug_text")
{
	SetBaseShader<ShaderDebug>();
}

void ShaderDebugText::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderDebug::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
}

bool ShaderDebugText::Draw(Anvil::Buffer &vertexBuffer,uint32_t vertexCount,Anvil::DescriptorSet &descSetTexture,const Mat4 &mvp,const Vector4 &color)
{
	assert(vertexCount <= umath::to_integral(GameLimits::MaxMeshVertices));
	if(vertexCount > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw debug mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return false;
	}
	return RecordBindDescriptorSet(descSetTexture) && ShaderDebug::Draw(vertexBuffer,vertexCount,mvp,color);
}

 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "c_shader_debug_text.h"
#include "cmaterialmanager.h"
#include "pragma/model/c_side.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(DebugText,debugtext);

extern DLLCLIENT CGame *c_game;

DebugText::DebugText()
	: Debug("debugtext","debug/vs_debug_text","debug/fs_debug_text")
{}

void DebugText::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Debug::InitializePipelineLayout(context,setLayouts,pushConstants);
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Texture
	}));
}

void DebugText::Draw(const Mat4 &matModel,Vulkan::DescriptorSetObject *descSet,Vulkan::BufferObject *vertexBuffer,uint32_t vertexCount,const Vulkan::DescriptorSet &texDescSet)
{
	assert(vertexCount <= umath::to_integral(GameLimits::MaxMeshVertices));
	if(vertexCount > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw debug mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return;
	}
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	//auto &pipeline = *GetPipeline();
	auto &layout = GetPipeline()->GetPipelineLayout();

	drawCmd->BindDescriptorSet(layout,descSet);
	drawCmd->BindDescriptorSet(1,layout,texDescSet);
	drawCmd->BindVertexBuffer({vertexBuffer,c_game->GetDummyVertexBuffer()});
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,16,16,&matModel);
	//drawCmd->BindIndexBuffer(indexBuffer,vk::IndexType::eUint16);
	//drawCmd->DrawIndexed(mesh->GetTriangleVertexCount());
	drawCmd->Draw(vertexCount,1);
}
#endif
