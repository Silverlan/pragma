#ifndef __C_SHADER_WATER_SURFACE_SUM_EDGES_HPP__
#define __C_SHADER_WATER_SURFACE_SUM_EDGES_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderWaterSurfaceSumEdges
		: public prosper::ShaderCompute
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_WATER;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_SURFACE_INFO;

		enum class WaterBinding : uint32_t
		{
			WaterParticles = 0u,
			WaterEdgeData
		};

		ShaderWaterSurfaceSumEdges(prosper::Context &context,const std::string &identifier);
		bool Compute(Anvil::DescriptorSet &descSetSurfaceInfo,Anvil::DescriptorSet &descSetEdges,uint32_t width,uint32_t length);
	protected:
		ShaderWaterSurfaceSumEdges(prosper::Context &context,const std::string &identifier,const std::string &csShader);
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "pragma/clientdefinitions.h"
#include "shadersystem.h"

namespace Shader
{
	class DLLCLIENT WaterSurfaceSumEdges
		: public Base
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			WaterParticles = 0,
			WaterEdgeData = WaterParticles,

			WaterSurfaceInfo = WaterParticles +1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			WarterParticles = 0,
			WaterEdgeData = WarterParticles +1,

			WaterSurfaceInfo = 0
		};
		WaterSurfaceSumEdges();
		void Compute(const Vulkan::DescriptorSetObject *descSetSurfaceInfo,const Vulkan::DescriptorSetObject *descSetEdges,uint32_t width,uint32_t length);
		static Vulkan::DescriptorSet CreateEdgeDescriptorSet();
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	};
};
#endif
#endif
