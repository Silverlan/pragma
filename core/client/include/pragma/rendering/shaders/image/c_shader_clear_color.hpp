/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_CLEAR_COLOR_HPP__
#define __C_SHADER_CLEAR_COLOR_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma {
	class DLLCLIENT ShaderClearColor : public prosper::ShaderGraphics {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

#pragma pack(push, 1)
		struct PushConstants {
			Vector4 clearColor;
		};
#pragma pack(pop)

		ShaderClearColor(prosper::IPrContext &context, const std::string &identifier);
		ShaderClearColor(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");

		bool RecordDraw(prosper::ShaderBindState &bindState, const PushConstants &pushConstants = {{0.f, 0.f, 0.f, 0.f}}) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
