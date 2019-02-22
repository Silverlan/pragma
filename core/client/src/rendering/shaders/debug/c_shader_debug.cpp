#include "stdafx_client.h"
#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"
#include <pragma/model/vertex.h>
#include <prosper_util_square_shape.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderDebug::VERTEX_BINDING_VERTEX) ShaderDebug::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderDebug::VERTEX_ATTRIBUTE_POSITION) ShaderDebug::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32_SFLOAT};

ShaderDebug::ShaderDebug(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: ShaderScene(context,identifier,vsShader,fsShader)
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}
ShaderDebug::ShaderDebug(prosper::Context &context,const std::string &identifier)
	: ShaderDebug(context,identifier,"debug/vs_debug","debug/fs_debug")
{}

bool ShaderDebug::ShouldInitializePipeline(uint32_t pipelineIdx) {return true;}

void ShaderDebug::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
	pipelineInfo.toggle_depth_bias(true,1.f,0.f,0.f);
	pipelineInfo.toggle_dynamic_states(true,{Anvil::DynamicState::DEPTH_BIAS});

	VERTEX_BINDING_VERTEX.stride = std::numeric_limits<decltype(VERTEX_BINDING_VERTEX.stride)>::max();
	switch(static_cast<Pipeline>(pipelineIdx))
	{
		case Pipeline::Triangle:
			break;
		case Pipeline::Line:
			pipelineInfo.toggle_dynamic_states(true,{Anvil::DynamicState::LINE_WIDTH});
			pipelineInfo.set_primitive_topology(Anvil::PrimitiveTopology::LINE_LIST);
		case Pipeline::Wireframe:
			prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo,Anvil::PolygonMode::LINE);
			break;
		case Pipeline::LineStrip:
			prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo,Anvil::PolygonMode::LINE);
			pipelineInfo.set_primitive_topology(Anvil::PrimitiveTopology::LINE_STRIP);
			pipelineInfo.toggle_dynamic_states(true,{Anvil::DynamicState::LINE_WIDTH});
			break;
		case Pipeline::Vertex:
			VERTEX_BINDING_VERTEX.stride = sizeof(Vertex);
		case Pipeline::Point:
			prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo,Anvil::PolygonMode::POINT);
			pipelineInfo.set_primitive_topology(Anvil::PrimitiveTopology::POINT_LIST);
			break;
	}

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::VERTEX_BIT);
}

bool ShaderDebug::BeginDraw(const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,Pipeline pipelineIdx)
{
	return ShaderGraphics::BeginDraw(cmdBuffer,umath::to_integral(pipelineIdx)) == true &&
		(*cmdBuffer)->record_set_depth_bias(1.f,0.f,0.f);
}

bool ShaderDebug::Draw(const std::vector<Anvil::Buffer*> &buffers,uint32_t vertexCount,const Mat4 &mvp,const Vector4 &color)
{
	assert(vertexCount <= umath::to_integral(GameLimits::MaxMeshVertices));
	if(vertexCount > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw debug mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return false;
	}

	PushConstants pushConstants {mvp,color};
	if(
		RecordBindVertexBuffers(buffers) == false ||
		RecordPushConstants(pushConstants) == false
	)
		return false;
	//c_engine->StartGPUTimer(GPUTimerEvent::DebugMesh); // prosper TODO
	auto r = RecordDraw(vertexCount);
	//c_engine->StopGPUTimer(GPUTimerEvent::DebugMesh); // prosper TODO
	return r;
}

bool ShaderDebug::Draw(Anvil::Buffer &vertexBuffer,uint32_t vertexCount,const Mat4 &mvp,const Vector4 &color)
{
	return Draw(std::vector<Anvil::Buffer*>{&vertexBuffer},vertexCount,mvp,color);
}

/////////////////////

decltype(ShaderDebugTexture::VERTEX_BINDING_VERTEX) ShaderDebugTexture::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderDebugTexture::VERTEX_ATTRIBUTE_POSITION) ShaderDebugTexture::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32_SFLOAT};
decltype(ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE) ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderDebugTexture::ShaderDebugTexture(prosper::Context &context,const std::string &identifier)
	: ShaderScene(context,identifier,"debug/vs_debug_uv","debug/fs_debug_texture")
{}

void ShaderDebugTexture::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,Anvil::CullModeFlagBits::NONE);
	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_TEXTURE);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderDebug::PushConstants),Anvil::ShaderStageFlagBits::VERTEX_BIT);
}
bool ShaderDebugTexture::Draw(Anvil::DescriptorSet &descSetTexture,const ShaderDebug::PushConstants &pushConstants)
{
	auto buf = prosper::util::get_square_vertex_buffer(c_engine->GetDevice());
	return RecordBindVertexBuffer(**buf) &&
		RecordBindDescriptorSet(descSetTexture,DESCRIPTOR_SET_TEXTURE.setIndex) &&
		RecordPushConstants(pushConstants) &&
		RecordDraw(prosper::util::get_square_vertex_count());
}

/////////////////////

decltype(ShaderDebugVertexColor::VERTEX_BINDING_COLOR) ShaderDebugVertexColor::VERTEX_BINDING_COLOR = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderDebugVertexColor::VERTEX_ATTRIBUTE_COLOR) ShaderDebugVertexColor::VERTEX_ATTRIBUTE_COLOR = {VERTEX_BINDING_COLOR,Anvil::Format::R32G32B32A32_SFLOAT};

ShaderDebugVertexColor::ShaderDebugVertexColor(prosper::Context &context,const std::string &identifier)
	: ShaderDebug(context,identifier,"debug/vs_debug_vertex_color","debug/fs_debug")
{
	SetBaseShader<ShaderDebug>();
}

void ShaderDebugVertexColor::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderDebug::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_COLOR);
}

bool ShaderDebugVertexColor::Draw(Anvil::Buffer &vertexBuffer,Anvil::Buffer &colorBuffer,uint32_t vertexCount,const Mat4 &modelMatrix)
{
	return ShaderDebug::Draw(std::vector<Anvil::Buffer*>{&vertexBuffer,&colorBuffer},vertexCount,modelMatrix);
}

 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/debug/c_shader_debug.h"
#include "cmaterialmanager.h"
#include "pragma/model/c_side.h"
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>

using namespace Shader;

LINK_SHADER_TO_CLASS(Debug,debug);
LINK_SHADER_TO_CLASS(DebugLine,debugline);
LINK_SHADER_TO_CLASS(DebugTriangleLine,debugtriangleline);
LINK_SHADER_TO_CLASS(DebugLineStrip,debuglinestrip);
LINK_SHADER_TO_CLASS(DebugPoint,debugpoint);
LINK_SHADER_TO_CLASS(DebugPointVertex,debugpointvertex);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

decltype(Debug::MAX_INSTANCE_COUNT) Debug::MAX_INSTANCE_COUNT = 32;

Debug::Debug(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: Base3D(identifier,vsShader,fsShader,gsShader)
{
	SetUseAlpha(true);
	SetUseDepth(true);
}

Debug::Debug()
	: Debug("debug","debug/vs_debug","debug/fs_debug")
{}

void Debug::InitializeDynamicStates(std::vector<vk::DynamicState> &states)
{
	Base3D::InitializeDynamicStates(states);
	states.push_back(vk::DynamicState::eDepthBias);
}

void Debug::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	Base3D::SetupPipeline(pipelineIdx,info);
	auto *rasterizationState = const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState);
	rasterizationState->depthBiasEnable = true;
	rasterizationState->depthBiasConstantFactor = 1.f;
	rasterizationState->depthBiasClamp = 0.f;
	rasterizationState->depthBiasSlopeFactor = 0.f;
}

void Debug::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::VERTEX_BIT} // Instance
	}));

	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::VERTEX_BIT,0,32 // Push Constants
	});
}
uint32_t Debug::GetVertexStride() const {return sizeof(Vector3);}
void Debug::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Base::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		static_cast<uint32_t>(Binding::Vertex),
		GetVertexStride(),
		Anvil::VertexInputRate::VERTEX
	});
	vertexBindingDescriptions.push_back({
		static_cast<uint32_t>(Binding::Color),
		sizeof(Vector4),
		Anvil::VertexInputRate::VERTEX
	});

	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::Vertex),
		static_cast<uint32_t>(Binding::Vertex),
		Anvil::Format::R32G32B32_SFLOAT,0
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::Color),
		static_cast<uint32_t>(Binding::Color),
		Anvil::Format::R32G32B32A32_SFLOAT,0
	});
}

bool Debug::BeginDraw(Camera &cam)
{
	if(Base3D::BeginDraw() == false)
		return false;
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();
	auto vp = cam.GetProjectionMatrix() *cam.GetViewMatrix();
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,16,&vp);
	drawCmd->SetDepthBias(1.f,0.f,0.f);
	//drawCmd->SetLineWidth(10.f);
	return true;
}
void Debug::Draw(const Mat4 &matModel,Vulkan::DescriptorSetObject *descSet,Vulkan::BufferObject *vertexBuffer,uint32_t vertexCount,Vulkan::BufferObject *colorBuffer)
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
	if(colorBuffer == nullptr)
		drawCmd->BindVertexBuffer({vertexBuffer,c_game->GetDummyVertexBuffer()});
	else
		drawCmd->BindVertexBuffer({vertexBuffer,colorBuffer});
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,16,16,&matModel);
	//drawCmd->BindIndexBuffer(indexBuffer,vk::IndexType::eUint16);
	//drawCmd->DrawIndexed(mesh->GetTriangleVertexCount());
	c_engine->StartGPUTimer(GPUTimerEvent::DebugMesh);
	drawCmd->Draw(vertexCount,1);
	c_engine->StopGPUTimer(GPUTimerEvent::DebugMesh);
}
void Debug::Draw(Vulkan::DescriptorSetObject *descSet,Vulkan::BufferObject *vertexBuffer,uint32_t vertexCount,Vulkan::BufferObject *colorBuffer)
{
	Draw(umat::identity(),descSet,vertexBuffer,vertexCount,colorBuffer);
}

/////////////////////////////////

void DebugLine::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	Debug::SetupPipeline(pipelineIdx,info);
	auto *assemblyState = const_cast<vk::PipelineInputAssemblyStateCreateInfo*>(info.pInputAssemblyState);
	assemblyState->topology = vk::PrimitiveTopology::eLineList;
	auto *rasterizationState = const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState);
	rasterizationState->polygonMode = Anvil::PolygonMode::LINE;
}

/////////////////////////////////

void DebugTriangleLine::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	DebugLine::SetupPipeline(pipelineIdx,info);
	auto *assemblyState = const_cast<vk::PipelineInputAssemblyStateCreateInfo*>(info.pInputAssemblyState);
	assemblyState->topology = vk::PrimitiveTopology::eTriangleList;
}

/////////////////////////////////

void DebugLineStrip::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	DebugLine::SetupPipeline(pipelineIdx,info);
	auto *assemblyState = const_cast<vk::PipelineInputAssemblyStateCreateInfo*>(info.pInputAssemblyState);
	assemblyState->topology = vk::PrimitiveTopology::eLineStrip;
	auto *rasterizationState = const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState);
	rasterizationState->polygonMode = Anvil::PolygonMode::LINE;
}

/////////////////////////////////

void DebugPoint::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	Debug::SetupPipeline(pipelineIdx,info);
	auto *assemblyState = const_cast<vk::PipelineInputAssemblyStateCreateInfo*>(info.pInputAssemblyState);
	assemblyState->topology = vk::PrimitiveTopology::ePointList;
	auto *rasterizationState = const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState);
	rasterizationState->polygonMode = Anvil::PolygonMode::POINT;
}

/////////////////////////////////

uint32_t DebugPointVertex::GetVertexStride() const {return sizeof(Vertex);}
#endif

