/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_CALC_IMAGE_COLOR_HPP__
#define __C_SHADER_CALC_IMAGE_COLOR_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderCalcImageColor
		: public prosper::ShaderCompute
	{
	public:
#pragma pack(push,1)
		struct PushConstants
		{
			int32_t sampleCount;
		};
#pragma pack(pop)

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_COLOR;

		ShaderCalcImageColor(prosper::Context &context,const std::string &identifier);
		bool Compute(prosper::IDescriptorSet &descSetTexture,prosper::IDescriptorSet &descSetColor,uint32_t sampleCount);
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif