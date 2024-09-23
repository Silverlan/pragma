/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_FORWARDP_LIGHT_INDEXING_HPP__
#define __C_SHADER_FORWARDP_LIGHT_INDEXING_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma {
	class DLLCLIENT ShaderForwardPLightIndexing : public prosper::ShaderCompute {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_VISIBLE_LIGHT;

#pragma pack(push, 1)
		struct PushConstants {
			uint32_t tileCount;
		};
#pragma pack(pop)

		ShaderForwardPLightIndexing(prosper::IPrContext &context, const std::string &identifier);
		bool RecordCompute(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetLights, uint32_t tileCount) const;
	  protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
