// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.pp_hdr;

export import :rendering.enums;
export import :rendering.shaders.base;

export namespace pragma {
	class DLLCLIENT ShaderPPHDR : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::Format RENDER_PASS_FORMAT;
		static prosper::Format RENDER_PASS_FORMAT_HDR;

		enum class Pipeline : uint32_t {
			LDR = 0,
			HDR,

			Count
		};

		enum class TextureBinding : uint32_t {
			Texture = 0u,
			Bloom,
		};

#pragma pack(push, 1)
		struct PushConstants {
			float exposure;
			float bloomScale;
			rendering::ToneMapping toneMapping;
			uint32_t flipVertically;
		};
#pragma pack(pop)

		ShaderPPHDR(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, rendering::ToneMapping toneMapping, float exposure, float bloomScale, bool flipVertically = false) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};
