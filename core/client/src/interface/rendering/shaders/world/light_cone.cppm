// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_light_cone;

export import :rendering.shaders.textured;

export namespace pragma {
	class DLLCLIENT ShaderLightCone : public ShaderGameWorldLightingPass {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_MAP;

#pragma pack(push, 1)
		struct PushConstants {
			float coneLength;
			uint32_t boundLightIndex;
			uint32_t resolution;
		};
#pragma pack(pop)

		ShaderLightCone(prosper::IPrContext &context, const std::string &identifier);
		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
	  protected:
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		int32_t m_boundLightIndex = -1;
	};
};
