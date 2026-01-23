// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.clear_color;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderClearColor : public prosper::ShaderGraphics {
	  public:
		static VertexBinding VERTEX_BINDING_VERTEX;
		static VertexAttribute VERTEX_ATTRIBUTE_POSITION;

#pragma pack(push, 1)
		struct PushConstants {
			Vector4 clearColor;
		};
#pragma pack(pop)

		ShaderClearColor(prosper::IPrContext &context, const std::string &identifier);
		ShaderClearColor(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");

		bool RecordDraw(prosper::ShaderBindState &bindState, const PushConstants &pushConstants = {{0.f, 0.f, 0.f, 0.f}}) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
