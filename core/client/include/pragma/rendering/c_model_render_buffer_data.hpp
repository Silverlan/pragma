/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_MODEL_RENDER_BUFFER_DATA_HPP__
#define __C_MODEL_RENDER_BUFFER_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include <prosper_context.hpp>
#include <material.h>

namespace prosper {
	class IRenderBuffer;
};
namespace pragma {
	enum class GameShaderSpecializationConstantFlag : uint32_t;
};
namespace pragma::rendering {
	struct DLLCLIENT RenderBufferData {
		enum class StateFlags : uint32_t {
			None = 0,
			EnableDepthPrepass = 1u,
			EnableGlowPass = EnableDepthPrepass<<1u,
		};
		std::shared_ptr<prosper::IRenderBuffer> renderBuffer;
		pragma::GameShaderSpecializationConstantFlag pipelineSpecializationFlags;
		msys::MaterialHandle material;
		StateFlags stateFlags = StateFlags::EnableDepthPrepass;

		void SetDepthPrepassEnabled(bool enabled);
		bool IsDepthPrepassEnabled() const;

		void SetGlowPassEnabled(bool enabled);
		bool IsGlowPassEnabled() const;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::RenderBufferData::StateFlags)

#endif
