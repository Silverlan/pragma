// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.particle_simulation;

export import pragma.prosper;

export namespace pragma::rendering::shaders {
	class DLLCLIENT ShaderParticleSimulation : public prosper::ShaderCompute {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			//int32_t sampleCount;
		};
#pragma pack(pop)

		ShaderParticleSimulation(prosper::IPrContext &context, const std::string &identifier);
		// bool RecordCompute(prosper::ShaderBindState &bindState) const;
	  protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
