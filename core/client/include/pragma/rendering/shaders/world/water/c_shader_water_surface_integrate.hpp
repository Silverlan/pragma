#ifndef __C_SHADER_WATER_SURFACE_INTEGRATE_HPP__
#define __C_SHADER_WATER_SURFACE_INTEGRATE_HPP__

#include "pragma/rendering/shaders/world/water/c_shader_water_surface.hpp"

namespace pragma
{
	class DLLCLIENT ShaderWaterSurfaceIntegrate
		: public ShaderWaterSurface
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_WATER_PARTICLES;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_SURFACE_INFO;

		enum class WaterParticlesBinding : uint32_t
		{
			WaterParticles = 0u
		};

		ShaderWaterSurfaceIntegrate(prosper::Context &context,const std::string &identifier);
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "pragma/clientdefinitions.h"
#include "shadersystem.h"

namespace Shader
{
	class DLLCLIENT WaterSurfaceIntegrate
		: public Base
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			WaterParticles = 0,

			WaterSurfaceInfo = WaterParticles +1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			WarterParticles = 0,

			WaterSurfaceInfo = 0
		};
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	public:
		WaterSurfaceIntegrate();
		void Compute(const Vulkan::DescriptorSetObject *descSetSurfaceInfo,const Vulkan::DescriptorSetObject *descSetIntegrate,uint32_t width,uint32_t length);
		static Vulkan::DescriptorSet CreateWaterParticleDescSet();
	};
};
#endif
#endif
