/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_BASE_CUBEMAP_HPP__
#define __C_SHADER_BASE_CUBEMAP_HPP__

#include "pragma/clientdefinitions.h"
#include <mathutil/umath.h>
#include <mathutil/umat.h>
#include <shader/prosper_shader.hpp>

namespace prosper {
	class RenderTarget;
	class Image;
	namespace util {
		struct TextureCreateInfo;
		struct SamplerCreateInfo;
	};
};
namespace pragma {
	class DLLCLIENT ShaderCubemap : public prosper::ShaderGraphics {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

#pragma pack(push, 1)
		struct PushConstants {
			Mat4 projection;
			Mat4 view;
		};
#pragma pack(pop)
	  protected:
		ShaderCubemap(prosper::IPrContext &context, const std::string &identifier, const std::string &vertexShader, const std::string &fragmentShader);
		ShaderCubemap(prosper::IPrContext &context, const std::string &identifier, const std::string &fragmentShader);
		std::shared_ptr<prosper::IBuffer> CreateCubeMesh(uint32_t &outNumVerts) const;
		std::shared_ptr<prosper::IImage> CreateCubeMap(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags = prosper::util::ImageCreateInfo::Flags::None) const;
		std::shared_ptr<prosper::RenderTarget> CreateCubeMapRenderTarget(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags = prosper::util::ImageCreateInfo::Flags::None) const;
		static void InitializeSamplerCreateInfo(prosper::util::ImageCreateInfo::Flags flags, prosper::util::SamplerCreateInfo &inOutSamplerCreateInfo);
		static void InitializeTextureCreateInfo(prosper::util::TextureCreateInfo &inOutTextureCreateInfo);
		const Mat4 &GetProjectionMatrix(float aspectRatio) const;
		const Mat4 &GetViewMatrix(uint8_t layerId) const;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};

#endif
