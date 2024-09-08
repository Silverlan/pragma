/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#ifndef __C_SHADER_MERGE_2D_IMAGE_INTO_EQUIRECTANGULAR_HPP__
#define __C_SHADER_MERGE_2D_IMAGE_INTO_EQUIRECTANGULAR_HPP__

#include "pragma/rendering/shaders/c_shader_base_cubemap.hpp"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {
	class Texture;
};
namespace pragma {
	class DLLCLIENT ShaderMerge2dImageIntoEquirectangular : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE_2D;

		enum class CubeFace : uint32_t {
			PositiveX = 0,
			NegativeX,
			PositiveY,
			NegativeY,
			PositiveZ,
			NegativeZ,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			float xFactor = 1.f;
			CubeFace cubeFace = CubeFace::PositiveX;
		};
#pragma pack(pop)

		ShaderMerge2dImageIntoEquirectangular(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTextureEquirect, prosper::IDescriptorSet &descSetTexture2d, CubeFace cubeFace = CubeFace::PositiveX, umath::Degree range = 360.f);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
