/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

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
