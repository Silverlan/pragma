// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.pp_fxaa;

export import :rendering.shaders.base;

export namespace pragma {
	class DLLCLIENT ShaderPPFXAA : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		enum class TextureBinding : uint32_t { SceneTexturePostToneMapping = 0, SceneTextureHdr };

#pragma pack(push, 1)
		struct DLLCLIENT PushConstants {
			// Required due to gcc bug (see https://stackoverflow.com/q/46866686)
			PushConstants() {};
			~PushConstants() {};

			float subPixelAliasingRemoval = 0.75f;
			float edgeThreshold = 0.166f;
			float minEdgeThreshold = 0.0833f;
		};
#pragma pack(pop)

		ShaderPPFXAA(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, const PushConstants &pushConstants = PushConstants {}) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};
