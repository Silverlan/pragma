// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.textured_alpha_transition;

export import :rendering.shaders.textured;

export namespace pragma {
	class DLLCLIENT ShaderTexturedAlphaTransition : public ShaderGameWorldLightingPass {
	  public:
		static ShaderGraphics::VertexBinding VERTEX_BINDING_ALPHA;
		static VertexAttribute VERTEX_ATTRIBUTE_ALPHA;

#pragma pack(push, 1)
		struct PushConstants {
			int32_t alphaCount;
		};
#pragma pack(pop)

		ShaderTexturedAlphaTransition(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
