// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_COMPOSE_RMA_HPP__
#define __C_SHADER_COMPOSE_RMA_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"

namespace prosper {
	class Image;
	class Texture;
};
namespace pragma {
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
		bool InsertAmbientOcclusion(prosper::IPrContext &context, const std::string &rmaInputPath, uimg::ImageBuffer &aoImgBuffer, const std::string *optRmaOutputPath = nullptr);
		bool InsertAmbientOcclusion(prosper::IPrContext &context, const std::string &rmaInputPath, prosper::IImage &aoImg, const std::string *optRmaOutputPath = nullptr);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderComposeRMA::Flags)

#endif
