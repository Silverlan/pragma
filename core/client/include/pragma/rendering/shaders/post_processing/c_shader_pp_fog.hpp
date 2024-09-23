/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PP_FOG_HPP__
#define __C_SHADER_PP_FOG_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"

namespace pragma {
	class DLLCLIENT ShaderPPFog : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_BUFFER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_FOG;

#pragma pack(push, 1)
		struct DLLCLIENT Fog {
			enum class DLLCLIENT Type : uint32_t { Linear = 0, Exponential, Exponential2 };
			enum class DLLCLIENT Flag : uint32_t { None = 0, Enabled = 1 };
			Vector4 color = {0.f, 0.f, 0.f, 0.f};
			float start = 0.f;
			float end = 0.f;
			float density = 0.f;
			Type type = Type::Linear;
			Flag flags = Flag::None;
		};
#pragma pack(pop)

		ShaderPPFog(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetDepth, prosper::IDescriptorSet &descSetCamera, prosper::IDescriptorSet &descSetFog) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
