// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.shaders.compose_rma;

export import pragma.image;
export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderComposeRMA : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		enum class TextureBinding : uint32_t {
			RoughnessMap = 0,
			MetalnessMap,
			AmbientOcclusionMap,

			Count
		};

		enum class Flags : uint32_t { None = 0, UseSpecularWorkflow = 1 };

#pragma pack(push, 1)
		struct PushConstants {
			Flags flags = Flags::None;
		};
#pragma pack(pop)

		ShaderComposeRMA(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::IImage> ComposeRMA(prosper::IPrContext &context, prosper::Texture *optRoughnessMap, prosper::Texture *optMetalnessMap, prosper::Texture *optAoMap, Flags flags = Flags::None);
		bool InsertAmbientOcclusion(prosper::IPrContext &context, const std::string &rmaInputPath, image::ImageBuffer &aoImgBuffer, const std::string *optRmaOutputPath = nullptr);
		bool InsertAmbientOcclusion(prosper::IPrContext &context, const std::string &rmaInputPath, prosper::IImage &aoImg, const std::string *optRmaOutputPath = nullptr);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::ShaderComposeRMA::Flags)
};
