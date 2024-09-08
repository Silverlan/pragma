/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PP_WATER_HPP__
#define __C_SHADER_PP_WATER_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"

namespace pragma {
	class DLLCLIENT ShaderPPWater : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_BUFFER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_REFRACTION_MAP;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_FOG;

#pragma pack(push, 1)
		struct PushConstants {
			Vector4 clipPlane;
		};
#pragma pack(pop)

		ShaderPPWater(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat);
		bool RecordRefractionMaterial(prosper::ShaderBindState &bindState, CMaterial &mat) const;
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetDepth, prosper::IDescriptorSet &descSetCamera, prosper::IDescriptorSet &descSetTime, prosper::IDescriptorSet &descSetFog, const Vector4 &clipPlane) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
