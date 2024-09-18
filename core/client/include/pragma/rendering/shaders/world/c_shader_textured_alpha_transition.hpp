/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_TEXTURED_ALPHA_TRANSITION_HPP__
#define __C_SHADER_TEXTURED_ALPHA_TRANSITION_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma {
	class DLLCLIENT ShaderTexturedAlphaTransition : public ShaderGameWorldLightingPass {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ALPHA;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ALPHA;

#pragma pack(push, 1)
		struct PushConstants {
			int32_t alphaCount;
		};
#pragma pack(pop)

		ShaderTexturedAlphaTransition(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
