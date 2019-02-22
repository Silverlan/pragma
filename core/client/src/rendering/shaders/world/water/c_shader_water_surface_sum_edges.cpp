#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_sum_edges.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

decltype(ShaderWaterSurfaceSumEdges::DESCRIPTOR_SET_WATER) ShaderWaterSurfaceSumEdges::DESCRIPTOR_SET_WATER = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Water particles
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Water edge data
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
decltype(ShaderWaterSurfaceSumEdges::DESCRIPTOR_SET_SURFACE_INFO) ShaderWaterSurfaceSumEdges::DESCRIPTOR_SET_SURFACE_INFO = {&ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO};
ShaderWaterSurfaceSumEdges::ShaderWaterSurfaceSumEdges(prosper::Context &context,const std::string &identifier,const std::string &csShader)
	: prosper::ShaderCompute(context,identifier,csShader)
{}
ShaderWaterSurfaceSumEdges::ShaderWaterSurfaceSumEdges(prosper::Context &context,const std::string &identifier)
	: ShaderWaterSurfaceSumEdges(context,identifier,"compute/water/cs_water_surface_sum_edges")
{}
void ShaderWaterSurfaceSumEdges::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_WATER);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SURFACE_INFO);
}
bool ShaderWaterSurfaceSumEdges::Compute(Anvil::DescriptorSet &descSetSurfaceInfo,Anvil::DescriptorSet &descSetEdges,uint32_t width,uint32_t length)
{
	return RecordBindDescriptorSets({&descSetEdges,&descSetSurfaceInfo},DESCRIPTOR_SET_WATER.setIndex) &&
		RecordDispatch(umath::ceil(width /8.f),umath::ceil(length /8.f),1);
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_sum_edges.hpp"

using namespace Shader;

LINK_SHADER_TO_CLASS(WaterSurfaceSumEdges,watersurfacesumedges);

WaterSurfaceSumEdges::WaterSurfaceSumEdges()
	: Base("watersurfacesumedges","compute/water/cs_water_surface_sum_edges")
{}

void WaterSurfaceSumEdges::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT}, // Water particles
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Water edge data
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Surface info
	}));
}

Vulkan::DescriptorSet WaterSurfaceSumEdges::CreateEdgeDescriptorSet()
{
	static auto shader = ShaderSystem::get_shader("watersurfacesumedges");
	if(!shader.IsValid())
		return nullptr;
	auto *textured = static_cast<TexturedBase3D*>(shader.get());
	auto *pipeline = textured->GetPipeline();
	if(pipeline != nullptr)
	{
		auto &layout = const_cast<Vulkan::ShaderPipeline*>(pipeline)->GetDescriptorSetLayout(umath::to_integral(DescSet::WaterParticles));
		auto &pool = pipeline->GetPipeline()->GetContext().GetDescriptorPool(layout->GetDescriptorType());
		auto descSet = pool->CreateDescriptorSet(layout);
		return descSet;
	}
	return nullptr;
}

void WaterSurfaceSumEdges::Compute(const Vulkan::DescriptorSetObject *descSetSurfaceInfo,const Vulkan::DescriptorSetObject *descSetEdges,uint32_t width,uint32_t length)
{
	auto &context = *m_context.get();
	auto &computeCmd = context.GetComputeCmd();
	auto &pipeline = *GetPipeline();
	auto &layout = pipeline.GetPipelineLayout();
	
	computeCmd->BindDescriptorSet(umath::to_integral(DescSet::WaterParticles),layout,descSetEdges);
	computeCmd->BindDescriptorSet(umath::to_integral(DescSet::WaterSurfaceInfo),layout,descSetSurfaceInfo);
	computeCmd->Dispatch(umath::ceil(width /8.f),umath::ceil(length /8.f),1);
}
#endif