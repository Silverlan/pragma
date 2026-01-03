// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.velocity_buffer;

export import :rendering.shaders.prepass;

export namespace pragma {
	class DLLCLIENT ShaderVelocityBuffer : public ShaderPrepassBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MOTION_BLUR;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_BONE_BUFFER;

#pragma pack(push, 1)
		struct MotionBlurPushConstants {
			static constexpr auto Offset = sizeof(PushConstants);
			Vector3 padding;
			Mat4 prevPose;
		};
#pragma pack(pop)

		ShaderVelocityBuffer(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
