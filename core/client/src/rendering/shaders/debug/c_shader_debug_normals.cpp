#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/debug/c_shader_debug_normals.h"
#include "cmaterialmanager.h"
#include "pragma/rendering/uniformbinding.h"
#include "pragma/model/c_side.h"
#include "pragma/model/c_modelmesh.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(DebugNormals,debugnormals);

extern DLLCLIENT CGame *c_game;

decltype(DebugNormals::MAX_INSTANCE_COUNT) DebugNormals::MAX_INSTANCE_COUNT = 32;
decltype(DebugNormals::PUSH_CONSTANT_COUNT) DebugNormals::PUSH_CONSTANT_COUNT = 1;

DebugNormals::DebugNormals()
	: TexturedBase3D("debugnormals","debug/vs_debug_normals","debug/fs_debug_normals","debug/gs_debug_normals")
{}
void DebugNormals::InitializeShaderPipelines(const Vulkan::Context &context)
{
	Base::InitializeShaderPipelines(context);

	auto &instanceBuffer = CBaseEntity::GetInstanceBuffer();
	auto &boneBuffer = CBaseEntity::GetInstanceBoneBuffer();
	auto it = s_descriptorCache.find(umath::to_integral(DescSet::Instance));
	auto *pipeline = GetPipeline();
	if(it != s_descriptorCache.end())
		pipeline->SetBuffer(umath::to_integral(DescSet::Instance),it->second,umath::to_integral(Binding::Instance));
	else
	{
		s_descriptorCache.insert(decltype(s_descriptorCache)::value_type(
			umath::to_integral(DescSet::Instance),
			pipeline->SetBuffer(umath::to_integral(DescSet::Instance),instanceBuffer,umath::to_integral(Binding::Instance))
		));
	}

	it = s_descriptorCache.find(umath::to_integral(DescSet::BoneMatrix));
	if(it != s_descriptorCache.end())
		pipeline->SetBuffer(umath::to_integral(DescSet::BoneMatrix),it->second,umath::to_integral(Binding::BoneMatrix));
	else
	{
		s_descriptorCache.insert(decltype(s_descriptorCache)::value_type(
			umath::to_integral(DescSet::Instance),
			pipeline->SetBuffer(umath::to_integral(DescSet::BoneMatrix),boneBuffer,umath::to_integral(Binding::BoneMatrix))
		));
	}
}
void DebugNormals::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::GEOMETRY_BIT,0,PUSH_CONSTANT_COUNT
	});

	setLayouts.reserve(3); // Should NOT exceed 8 descriptor sets, since some older Nvidia GPUs (e.g. GTX 650 Ti BOOST) do not support more than that!
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Instance
		{Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::VERTEX_BIT} // Bone Matrices
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT} // Camera
	}));
}
void DebugNormals::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Base::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		static_cast<uint32_t>(Binding::Vertex),
		sizeof(Vertex),
		Anvil::VertexInputRate::VERTEX
	});
	vertexBindingDescriptions.push_back({
		static_cast<uint32_t>(Binding::BoneWeight),
		sizeof(VertexWeight),
		Anvil::VertexInputRate::VERTEX
	});

	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::Vertex),
		static_cast<uint32_t>(Binding::Vertex),
		Anvil::Format::R32G32B32_SFLOAT,0
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::Normal),
		static_cast<uint32_t>(Binding::Normal),
		Anvil::Format::R32G32B32_SFLOAT,sizeof(Vector3) +sizeof(Vector2)
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::Tangent),
		static_cast<uint32_t>(Binding::Tangent),
		Anvil::Format::R32G32B32_SFLOAT,sizeof(Vector3) +sizeof(Vector2) +sizeof(Vector3)
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::BiTangent),
		static_cast<uint32_t>(Binding::BiTangent),
		Anvil::Format::R32G32B32_SFLOAT,sizeof(Vector3) +sizeof(Vector2) +sizeof(Vector3) +sizeof(Vector3)
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::BoneWeightId),
		static_cast<uint32_t>(Binding::BoneWeightId),
		vk::Format::eR32G32B32A32Sint,0
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::BoneWeight),
		static_cast<uint32_t>(Binding::BoneWeight),
		Anvil::Format::R32G32B32A32_SFLOAT,sizeof(Vector4i)
	});
}

bool DebugNormals::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	return BeginDraw(cmdBuffer,RenderMode::Normals);
}

bool DebugNormals::BeginDraw(RenderMode mode)
{
	if(!m_context.IsValid())
		return false;
	auto &context = *m_context.get();
	return BeginDraw(context.GetDrawCmd(),mode);
}

bool DebugNormals::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,RenderMode mode)
{
	if(Base3D::BeginDraw(cmdBuffer) == false)
		return false;
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();
	Vulkan::Std140LayoutBlockData data{1};
	data<<static_cast<int32_t>(mode);
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::GEOMETRY_BIT,static_cast<uint32_t>(data.GetCount()),data.GetData());
	return true;
}
void DebugNormals::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	Base3D::SetupPipeline(pipelineIdx,info);
	/*auto *assemblyState = const_cast<vk::PipelineInputAssemblyStateCreateInfo*>(info.pInputAssemblyState());
	assemblyState->topology(vk::PrimitiveTopology::ePointList);
	auto *rasterizationState = const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState());
	rasterizationState->polygonMode(Anvil::PolygonMode::POINT);*/
}
#endif