// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.gradient;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderGradient : public prosper::ShaderBaseImageProcessing {
	  public:
		static const uint32_t MAX_GRADIENT_NODES = 3u;
#pragma pack(push, 1)
		struct Node {
			Node(const Vector4 &color = {}, float offset = 0.f) : color(color), offset(offset) {}
			Vector4 color = {};
			float offset = 0.f;
			std::array<uint8_t, 12> padding;
		};
		struct PushConstants {
			Vector2i textureSize;
			Vector2 boxIntersection;
			int32_t nodeCount;
			std::array<uint8_t, 12> padding;
			std::array<Node, MAX_GRADIENT_NODES> nodes;
		};
#pragma pack(pop)

		ShaderGradient(prosper::IPrContext &context, const std::string &identifier);
		virtual ~ShaderGradient() override;
		bool RecordDraw(prosper::ShaderBindState &bindState, const PushConstants &pushConstants) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};

	DLLCLIENT bool record_draw_gradient(prosper::IPrContext &context, const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer, prosper::RenderTarget &rt, const Vector2 &dir, const std::vector<ShaderGradient::Node> &nodes);
};
