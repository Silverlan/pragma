/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_HDR_HPP__
#define __C_SHADER_HDR_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace pragma {
	class DLLCLIENT ShaderHDR : public prosper::ShaderBaseImageProcessing {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			float exposure;
		};
#pragma pack(pop)

		ShaderHDR(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, float exposure) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
