// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_water_surface;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderWaterSurface : public prosper::ShaderCompute {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_WATER_EFFECT;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SURFACE_INFO;

		enum class WaterEffectBinding : uint32_t { WaterParticles = 0u, WaterPositions, TemporaryParticleHeights };
		enum class SurfaceInfoBinding : uint32_t { SurfaceInfo = 0u };

		ShaderWaterSurface(prosper::IPrContext &context, const std::string &identifier);
		bool RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetSurfaceInfo, prosper::IDescriptorSet &descSetParticles, uint32_t width, uint32_t length) const;
	  protected:
		ShaderWaterSurface(prosper::IPrContext &context, const std::string &identifier, const std::string &csShader);
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
