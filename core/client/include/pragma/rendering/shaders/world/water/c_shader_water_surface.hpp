#ifndef __C_SHADER_WATER_SURFACE_HPP__
#define __C_SHADER_WATER_SURFACE_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderWaterSurface
		: public prosper::ShaderCompute
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_WATER_EFFECT;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_SURFACE_INFO;

		enum class WaterEffectBinding : uint32_t
		{
			WaterParticles = 0u,
			WaterPositions,
			TemporaryParticleHeights
		};
		enum class SurfaceInfoBinding : uint32_t
		{
			SurfaceInfo = 0u
		};

		ShaderWaterSurface(prosper::Context &context,const std::string &identifier);
		bool Compute(Anvil::DescriptorSet &descSetSurfaceInfo,Anvil::DescriptorSet &descSetParticles,uint32_t width,uint32_t length);
	protected:
		ShaderWaterSurface(prosper::Context &context,const std::string &identifier,const std::string &csShader);
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "shadersystem.h"

namespace Shader
{
	class DLLCLIENT WaterSurface
		: public Base
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			WaterParticles = 0,
			WaterPositions = WaterParticles,
			TmpParticleHeights = WaterPositions,

			WaterSurfaceInfo = WaterPositions +1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			WarterParticles = 0,
			WaterPositions = WarterParticles +1,
			TmpParticleHeights = WaterPositions +1,

			WaterSurfaceInfo = 0
		};
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	public:
		WaterSurface();
		void Compute(const Vulkan::DescriptorSetObject *descSetSurfaceInfo,const Vulkan::DescriptorSetObject *descParticles,uint32_t width,uint32_t length);
		static Vulkan::DescriptorSet CreateParticleDescriptorSet();
		static Vulkan::DescriptorSet CreateSurfaceInfoDescriptorSet();
	};
};
#endif
#endif
