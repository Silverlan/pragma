// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.calc_image_color;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderCalcImageColor : public prosper::ShaderCompute {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			int32_t sampleCount;
		};
#pragma pack(pop)

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_COLOR;

		ShaderCalcImageColor(prosper::IPrContext &context, const std::string &identifier);
		bool RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetColor, uint32_t sampleCount) const;
	  protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
