// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_water_splash;

export import pragma.prosper;
export import pragma.shared;

export namespace pragma {
	class DLLCLIENT ShaderWaterSplash : public prosper::ShaderCompute {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_WATER_EFFECT;
		enum class WaterEffectBinding : uint32_t { WaterParticles = 0u, WaterPositions };

		ShaderWaterSplash(prosper::IPrContext &context, const std::string &identifier);
		bool RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descParticles, const physics::PhysWaterSurfaceSimulator::SplashInfo &info) const;
	  protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
