#ifndef __C_SHADER_WATER_SURFACE_SOLVE_EDGES_HPP__
#define __C_SHADER_WATER_SURFACE_SOLVE_EDGES_HPP__

#include "pragma/rendering/shaders/world/water/c_shader_water_surface_sum_edges.hpp"

namespace pragma
{
	class DLLCLIENT ShaderWaterSurfaceSolveEdges
		: public ShaderWaterSurfaceSumEdges
	{
	public:
		ShaderWaterSurfaceSolveEdges(prosper::Context &context,const std::string &identifier);
	};
};

// prosper TODO
#if 0
#include "pragma/clientdefinitions.h"
#include "shadersystem.h"

namespace Shader
{
	class DLLCLIENT WaterSurfaceSolveEdges
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
		WaterSurfaceSolveEdges();
		void Compute(const Vulkan::DescriptorSetObject *descSetSurfaceInfo,const Vulkan::DescriptorSetObject *descSetEdges,uint32_t width,uint32_t length);
		static Vulkan::DescriptorSet CreateEdgeDescriptorSet();
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	};
};
#endif
#endif
