#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_integrate.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

decltype(ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES) ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Water particles
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
decltype(ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_SURFACE_INFO) ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_SURFACE_INFO = {&ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO};
ShaderWaterSurfaceIntegrate::ShaderWaterSurfaceIntegrate(prosper::Context &context,const std::string &identifier)
	: ShaderWaterSurface(context,identifier,"compute/water/cs_water_surface_integrate")
{}
void ShaderWaterSurfaceIntegrate::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_WATER_PARTICLES);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SURFACE_INFO);
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_integrate.hpp"

using namespace Shader;

LINK_SHADER_TO_CLASS(WaterSurfaceIntegrate,watersurfaceintegrate);

WaterSurfaceIntegrate::WaterSurfaceIntegrate()
	: Base("watersurfaceintegrate","compute/water/cs_water_surface_integrate")
{}

void WaterSurfaceIntegrate::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Water particles
	}));
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Surface info
	}));
}

Vulkan::DescriptorSet WaterSurfaceIntegrate::CreateWaterParticleDescSet()
{
	static auto shader = ShaderSystem::get_shader("watersurfaceintegrate");
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

void WaterSurfaceIntegrate::Compute(const Vulkan::DescriptorSetObject *descSetSurfaceInfo,const Vulkan::DescriptorSetObject *descSetIntegrate,uint32_t width,uint32_t length)
{
	//Con::cerr<<"Compute: "<<width<<"x"<<length<<Con::endl; // 181x267
	auto &context = *m_context.get();
	auto &computeCmd = context.GetComputeCmd();
	auto &pipeline = *GetPipeline();
	auto &layout = pipeline.GetPipelineLayout();
	
	computeCmd->BindDescriptorSet(umath::to_integral(DescSet::WaterParticles),layout,descSetIntegrate);
	computeCmd->BindDescriptorSet(umath::to_integral(DescSet::WaterSurfaceInfo),layout,descSetSurfaceInfo);
	computeCmd->Dispatch(umath::ceil(width /8.f),umath::ceil(length /8.f),1);//width,length); // TODO
}
#endif