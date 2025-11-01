// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.shaders.world_noop;

export import :rendering.shaders.world_unlit;

export namespace pragma {
	class DLLCLIENT ShaderNoop : public ShaderUnlit {
	  public:
		ShaderNoop(prosper::IPrContext &context, const std::string &identifier);
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, msys::CMaterial &mat) const override;
		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const override;
	};
};
