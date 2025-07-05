// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_NOOP_HPP__
#define __C_SHADER_NOOP_HPP__

#include "pragma/rendering/shaders/world/c_shader_unlit.hpp"
#include <mathutil/uvec.h>

namespace pragma {
	class DLLCLIENT ShaderNoop : public ShaderUnlit {
	  public:
		ShaderNoop(prosper::IPrContext &context, const std::string &identifier);
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const override;
		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
	};
};

#endif
