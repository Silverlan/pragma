#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_splash.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

decltype(ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT) ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Water particles
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Water Positions
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
ShaderWaterSplash::ShaderWaterSplash(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderCompute(context,identifier,"compute/water/cs_water_splash")
{}

void ShaderWaterSplash::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PhysWaterSurfaceSimulator::SplashInfo),Anvil::ShaderStageFlagBits::COMPUTE_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_WATER_EFFECT);
}

bool ShaderWaterSplash::Compute(Anvil::DescriptorSet &descParticles,const PhysWaterSurfaceSimulator::SplashInfo &info)
{
	return RecordPushConstants(info) &&
		RecordBindDescriptorSet(descParticles,DESCRIPTOR_SET_WATER_EFFECT.setIndex) &&
		RecordDispatch(umath::ceil(info.width /8.f),umath::ceil(info.length /8.f),1);//width,length); // TODO
}

 // prosper TODO
#if 0
#include "pragma/rendering/shaders/world/water/c_shader_water_splash.hpp"

using namespace Shader;

LINK_SHADER_TO_CLASS(WaterSplash,watersplash);

WaterSplash::WaterSplash()
	: Base("watersplash","compute/water/cs_water_splash")
{}

void WaterSplash::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants = {
		{Anvil::ShaderStageFlagBits::COMPUTE_BIT,0,sizeof(PhysWaterSurfaceSimulator::SplashInfo) /sizeof(uint32_t)}
	};

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT}, // Water particles
		{Anvil::DescriptorType::STORAGE_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Water Positions
	}));
}

void WaterSplash::Compute(const Vulkan::DescriptorSetObject *descParticles,const PhysWaterSurfaceSimulator::SplashInfo &info)
{
	//Con::cerr<<"Compute: "<<width<<"x"<<length<<Con::endl; // 181x267
	auto &context = *m_context.get();
	auto &computeCmd = context.GetComputeCmd();
	auto &pipeline = *GetPipeline();
	auto &layout = pipeline.GetPipelineLayout();

	computeCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::COMPUTE_BIT,sizeof(info) /sizeof(uint32_t),&info);
	computeCmd->BindDescriptorSet(umath::to_integral(DescSet::WaterParticles),layout,descParticles);
	computeCmd->Dispatch(umath::ceil(info.width /8.f),umath::ceil(info.length /8.f),1);//width,length); // TODO
}
#endif