// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_WIREFRAME_HPP__
#define __C_SHADER_WIREFRAME_HPP__

#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"

namespace pragma {
	class DLLCLIENT ShaderWireframe : public ShaderPBR {
	  public:
		ShaderWireframe(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
	};
};

#endif
