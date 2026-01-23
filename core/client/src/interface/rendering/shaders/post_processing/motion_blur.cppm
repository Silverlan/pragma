// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.pp_motion_blur;

export import :rendering.shaders.base;

export namespace pragma {
	namespace shaderPPMotionBlur {
		using namespace shaderPPBase;
	}
	class DLLCLIENT ShaderPPMotionBlur : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE_VELOCITY;

#pragma pack(push, 1)
		struct DLLCLIENT PushConstants {
			float velocityScale;
			uint32_t blurQuality;
		};
#pragma pack(pop)

		ShaderPPMotionBlur(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, const PushConstants &pushConstants, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetTextureVelocity) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
