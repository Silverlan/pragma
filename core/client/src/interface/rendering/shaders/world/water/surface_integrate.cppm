// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_water_surface_integrate;

export import :rendering.shaders.world_water_surface;

export namespace pragma {
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
