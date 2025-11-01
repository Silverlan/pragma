// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.model_render_buffer_data;

export import :rendering.shaders.textured_enums;
export import pragma.cmaterialsystem;

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
	using namespace umath::scoped_enum::bitwise;
};
export {
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<pragma::rendering::RenderBufferData::StateFlags> : std::true_type {};
	}
};
