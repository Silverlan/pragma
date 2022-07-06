/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PP_MOTION_BLUR_HPP__
#define __C_SHADER_PP_MOTION_BLUR_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"

namespace pragma
{
	class DLLCLIENT ShaderPPMotionBlur
		: public ShaderPPBase
	{
	public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE_VELOCITY;

		ShaderPPMotionBlur(prosper::IPrContext &context,const std::string &identifier);
		bool RecordDraw(
			prosper::ShaderBindState &bindState,prosper::IDescriptorSet &descSetTexture,
			prosper::IDescriptorSet &descSetTextureVelocity
		) const;
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
