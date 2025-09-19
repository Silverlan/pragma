// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <prosper_context.hpp>
#include <material.h>

export module pragma.client.rendering.model_render_buffer_data;

export namespace pragma::rendering {
	struct DLLCLIENT RenderBufferData {
		enum class StateFlags : uint32_t {
			None = 0,
			EnableDepthPrepass = 1u,
			EnableGlowPass = EnableDepthPrepass << 1u,
			ExcludeFromAccelerationStructures = EnableGlowPass << 1u,
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
export {
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::RenderBufferData::StateFlags)
};
