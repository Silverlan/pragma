// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client.rendering.shaders:pp_dof;

export namespace pragma {
	class DLLCLIENT ShaderPPDoF : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_BUFFER;

		enum class Flags : uint32_t {
			None = 0,
			DebugShowFocus = 1,
			EnableVignette = DebugShowFocus << 1u,
			PentagonBokehShape = EnableVignette << 1u,
			DebugShowDepth = PentagonBokehShape << 1u,
		};

		enum class TextureBinding : uint32_t { SceneTexturePostToneMapping = 0, SceneTextureHdr };

#pragma pack(push, 1)
		struct DLLCLIENT PushConstants {
			Mat4 mvp;
			uint32_t width;
			uint32_t height;

			float focalDepth;
			float focalLength;
			float fstop;

			float zNear;
			float zFar;

			Flags flags;
			int32_t rings;
			int32_t ringSamples;
			float CoC;
			float maxBlur;
			float dither;
			float vignIn;
			float vignOut;
			float pentagonShapeFeather;
		};
#pragma pack(pop)

		ShaderPPDoF(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetDepth, const PushConstants &pushConstants = PushConstants {}) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};

export {
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderPPDoF::Flags)
};
