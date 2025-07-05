// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_WATER_SURFACE_INTEGRATE_HPP__
#define __C_SHADER_WATER_SURFACE_INTEGRATE_HPP__

#include "pragma/rendering/shaders/world/water/c_shader_water_surface.hpp"

namespace pragma {
	class DLLCLIENT ShaderWaterSurfaceIntegrate : public ShaderWaterSurface {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_WATER_PARTICLES;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SURFACE_INFO;

		enum class WaterParticlesBinding : uint32_t { WaterParticles = 0u };

		ShaderWaterSurfaceIntegrate(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
