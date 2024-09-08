/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#ifndef __C_SHADER_MERGE_IMAGES_HPP__
#define __C_SHADER_MERGE_IMAGES_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace pragma {
	class DLLCLIENT ShaderMergeImages : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE_2D;

		ShaderMergeImages(prosper::IPrContext &context, const std::string &identifier);
		virtual ~ShaderMergeImages() override;
		bool RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetTexture2) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
