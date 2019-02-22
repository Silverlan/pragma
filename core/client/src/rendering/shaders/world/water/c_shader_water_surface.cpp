#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

decltype(ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT) ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Water particles
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Water Positions
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Temporary Particle Heights
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
decltype(ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO) ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Surface info
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
ShaderWaterSurface::ShaderWaterSurface(prosper::Context &context,const std::string &identifier,const std::string &csShader)
	: prosper::ShaderCompute(context,identifier,csShader)
{}
ShaderWaterSurface::ShaderWaterSurface(prosper::Context &context,const std::string &identifier)
	: ShaderWaterSurface(context,identifier,"compute/water/cs_water_surface")
{}
void ShaderWaterSurface::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_WATER_EFFECT);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SURFACE_INFO);
}
bool ShaderWaterSurface::Compute(Anvil::DescriptorSet &descSetSurfaceInfo,Anvil::DescriptorSet &descSetParticles,uint32_t width,uint32_t length)
{
	return RecordBindDescriptorSets({&descSetParticles,&descSetSurfaceInfo},DESCRIPTOR_SET_WATER_EFFECT.setIndex) &&
		RecordDispatch(umath::ceil(width /8.f),umath::ceil(length /8.f),1);//width,length); // TODO
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/world/water/c_shader_water_surface.hpp"

using namespace Shader;

LINK_SHADER_TO_CLASS(WaterSurface,watersurface);

WaterSurface::WaterSurface()
	: Base("watersurface","compute/water/cs_water_surface")
{}

void WaterSurface::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

}

Vulkan::DescriptorSet WaterSurface::CreateParticleDescriptorSet()
{
	static auto shader = ShaderSystem::get_shader("watersurface");
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

Vulkan::DescriptorSet WaterSurface::CreateSurfaceInfoDescriptorSet()
{
	static auto shader = ShaderSystem::get_shader("watersurface");
	if(!shader.IsValid())
		return nullptr;
	auto *textured = static_cast<TexturedBase3D*>(shader.get());
	auto *pipeline = textured->GetPipeline();
	if(pipeline != nullptr)
	{
		auto &layout = const_cast<Vulkan::ShaderPipeline*>(pipeline)->GetDescriptorSetLayout(umath::to_integral(DescSet::WaterSurfaceInfo));
		auto &pool = pipeline->GetPipeline()->GetContext().GetDescriptorPool(layout->GetDescriptorType());
		auto descSet = pool->CreateDescriptorSet(layout);
		return descSet;
	}
	return nullptr;
}

void WaterSurface::Compute(const Vulkan::DescriptorSetObject *descSetSurfaceInfo,const Vulkan::DescriptorSetObject *descParticles,uint32_t width,uint32_t length)
{
	//Con::cerr<<"Compute: "<<width<<"x"<<length<<Con::endl; // 181x267
	auto &context = *m_context.get();
	auto &computeCmd = context.GetComputeCmd();
	auto &pipeline = *GetPipeline();
	auto &layout = pipeline.GetPipelineLayout();
	

}
#endif