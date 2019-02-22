#ifndef __C_SHADER_WATER_SPLASH_HPP__
#define __C_SHADER_WATER_SPLASH_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>
#include <pragma/physics/phys_water_surface_simulator.hpp>

namespace pragma
{
	class DLLCLIENT ShaderWaterSplash
		: public prosper::ShaderCompute
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_WATER_EFFECT;
		enum class WaterEffectBinding : uint32_t
		{
			WaterParticles = 0u,
			WaterPositions
		};

		ShaderWaterSplash(prosper::Context &context,const std::string &identifier);
		bool Compute(Anvil::DescriptorSet &descParticles,const PhysWaterSurfaceSimulator::SplashInfo &info);
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "pragma/clientdefinitions.h"
#include "shadersystem.h"
#include <pragma/physics/phys_water_surface_simulator.hpp>

namespace Shader
{
	class DLLCLIENT WaterSplash
		: public Base
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			WaterParticles = 0,
			WaterPositions = WaterParticles
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			WarterParticles = 0,
			WaterPositions = WarterParticles +1
		};
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	public:
		WaterSplash();
		void Compute(const Vulkan::DescriptorSetObject *descParticles,const PhysWaterSurfaceSimulator::SplashInfo &info);
	};
};
#endif

#endif
