// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.combine_image_channels;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderCombineImageChannels : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		enum class TextureBinding : uint32_t {
			ChannelR,
			ChannelG,
			ChannelB,
			ChannelA,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			uint32_t redChannel = 0;
			uint32_t greenChannel = 1;
			uint32_t blueChannel = 2;
			uint32_t alphaChannel = 3;
		};
#pragma pack(pop)

		ShaderCombineImageChannels(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::Texture> CombineImageChannels(prosper::IPrContext &context, prosper::Texture &channelR, prosper::Texture &channelG, prosper::Texture &channelB, prosper::Texture &channelA, const PushConstants &pushConstants);
	  protected:
		std::shared_ptr<prosper::RenderTarget> CreateRenderTarget(prosper::IPrContext &context, prosper::Extent2D extents) const;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
