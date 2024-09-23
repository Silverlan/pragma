/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_SSAO_HPP__
#define __C_SHADER_SSAO_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {
	class Texture;
	class IBuffer;
	class IDescriptorSetGroup;
};
namespace pragma {
	class CSceneComponent;
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
		bool RecordDraw(prosper::ShaderBindState &bindState, const pragma::CSceneComponent &scene, prosper::IDescriptorSet &descSetPrepass, const std::array<uint32_t, 2> &renderTargetDimensions) const;
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

#endif
