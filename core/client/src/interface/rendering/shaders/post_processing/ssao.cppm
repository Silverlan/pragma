// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.ssao;

export import pragma.prosper;

export namespace pragma {
	class CSceneComponent;
};
export namespace pragma {
	class DLLCLIENT ShaderSSAO : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::Format RENDER_PASS_FORMAT;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PREPASS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_NOISE_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SAMPLE_BUFFER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;

		enum class PrepassBinding : uint32_t { NormalBuffer = 0u, DepthBuffer };

#pragma pack(push, 1)
		struct PushConstants {
			std::array<uint32_t, 2> renderTargetDimensions;
		};
#pragma pack(pop)

		ShaderSSAO(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, const CSceneComponent &scene, prosper::IDescriptorSet &descSetPrepass, const std::array<uint32_t, 2> &renderTargetDimensions) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void OnPipelineInitialized(uint32_t pipelineIdx) override;
	  private:
		std::shared_ptr<prosper::IBuffer> m_kernelBuffer = nullptr;
		std::shared_ptr<prosper::Texture> m_noiseTexture = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupKernel = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupTexture = nullptr;
	};
};
