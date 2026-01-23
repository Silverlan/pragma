// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.forwardp_light_indexing;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderForwardPLightIndexing : public prosper::ShaderCompute {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_VISIBLE_LIGHT;

#pragma pack(push, 1)
		struct PushConstants {
			uint32_t tileCount;
		};
#pragma pack(pop)

		ShaderForwardPLightIndexing(prosper::IPrContext &context, const std::string &identifier);
		bool RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetLights, uint32_t tileCount) const;
	  protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
