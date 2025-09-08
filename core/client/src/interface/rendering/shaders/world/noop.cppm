// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/rendering/shaders/world/c_shader_unlit.hpp"
#include <mathutil/uvec.h>

export module pragma.client.rendering.shaders:world_noop;

export namespace pragma {
	class DLLCLIENT ShaderNoop : public ShaderUnlit {
	  public:
		ShaderNoop(prosper::IPrContext &context, const std::string &identifier);
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const override;
		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
	};
};
