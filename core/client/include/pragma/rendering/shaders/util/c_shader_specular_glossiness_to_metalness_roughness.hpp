/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_SPECULAR_GLOSSINESS_TO_METALNESS_ROUGHNESS_HPP__
#define __C_SHADER_SPECULAR_GLOSSINESS_TO_METALNESS_ROUGHNESS_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {
	class Image;
	class Texture;
};
namespace pragma {
	class DLLCLIENT ShaderSpecularGlossinessToMetalnessRoughness : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		enum class TextureBinding : uint32_t {
			DiffuseMap = 0,
			SpecularGlossinessMap,
			AmbientOcclusionMap,

			Count
		};

		struct DLLCLIENT MetalnessRoughnessImageSet {
			std::shared_ptr<prosper::IImage> albedoMap = nullptr;
			std::shared_ptr<prosper::IImage> rmaMap = nullptr;
		};

		enum class Pass : uint32_t { Albedo = 0, RMA };

#pragma pack(push, 1)
		struct PushConstants {

			PushConstants() {};
			Vector4 diffuseFactor = {1.f, 1.f, 1.f, 1.f};
			Vector4 specularFactor = {1.f, 1.f, 1.f, 1.f}; // Alpha is glossiness factor
			Pass pass = Pass::Albedo;
		};
#pragma pack(pop)

		ShaderSpecularGlossinessToMetalnessRoughness(prosper::IPrContext &context, const std::string &identifier);
		std::optional<MetalnessRoughnessImageSet> ConvertToMetalnessRoughness(prosper::IPrContext &context, prosper::Texture *optDiffuseMap, prosper::Texture *optSpecularGlossinessMap, const PushConstants &pushConstants = {}, prosper::Texture *optAoMap = nullptr);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
