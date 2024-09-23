/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_VELOCITY_BUFFER_HPP__
#define __C_SHADER_VELOCITY_BUFFER_HPP__

#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"

class Texture;
namespace pragma {
	class DLLCLIENT ShaderVelocityBuffer : public ShaderPrepassBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MOTION_BLUR;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_BONE_BUFFER;

#pragma pack(push, 1)
		struct MotionBlurPushConstants {
			static constexpr auto Offset = sizeof(ShaderPrepassBase::PushConstants);
			Vector3 padding;
			Mat4 prevPose;
		};
#pragma pack(pop)

		ShaderVelocityBuffer(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
