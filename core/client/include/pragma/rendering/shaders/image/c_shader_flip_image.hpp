/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#ifndef __C_SHADER_FLIP_IMAGE_HPP__
#define __C_SHADER_FLIP_IMAGE_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace pragma {
	class DLLCLIENT ShaderFlipImage : public prosper::ShaderBaseImageProcessing {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			uint32_t flipHorizontally;
			uint32_t flipVertically;
		};
#pragma pack(pop)

		ShaderFlipImage(prosper::IPrContext &context, const std::string &identifier);
		virtual ~ShaderFlipImage() override;
		bool RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTexture, bool flipHorizontally, bool flipVertically) const;
	  protected:
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, const PushConstants &pushConstants) const;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
