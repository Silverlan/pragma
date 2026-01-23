// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.cubemap_to_equirectangular;

export import :rendering.shaders.cubemap;

export namespace pragma {
	namespace shaderCubemapToEquirectangular {
		using namespace prosper::shaderBaseImageProcessing;
	}
	class DLLCLIENT ShaderCubemapToEquirectangular : public prosper::ShaderBaseImageProcessing {
	  public:
		enum class Pipeline : uint32_t {
			RGBA16 = 0,
			RGBA8,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			float xFactor = 1.f;
		};
#pragma pack(pop)

		ShaderCubemapToEquirectangular(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::Texture> CubemapToEquirectangularTexture(prosper::Texture &cubemap, uint32_t width = 1'600, uint32_t height = 800, math::Degree range = 360.f, prosper::ImageLayout cubemapLayout = prosper::ImageLayout::ShaderReadOnlyOptimal);
	  protected:
		std::shared_ptr<prosper::IImage> CreateEquirectangularMap(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags, bool hdr) const;
		std::shared_ptr<prosper::RenderTarget> CreateEquirectangularRenderTarget(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags, bool hdr) const;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
