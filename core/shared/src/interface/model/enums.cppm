// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "util_enum_flags.hpp"

export module pragma.shared:model.enums;

import pragma.math;

export {
	namespace pragma::geometry {
		enum class IndexType : uint8_t { UInt16 = 0u, UInt32 };
	}
	namespace pragma::asset {
		enum class ModelUpdateFlags : uint32_t {
			None = 0,
			UpdateBounds = 1,
			UpdatePrimitiveCounts = UpdateBounds << 1,
			InitializeCollisionShapes = UpdatePrimitiveCounts << 1,
			CalculateTangents = InitializeCollisionShapes << 1,
			UpdateVertexBuffer = CalculateTangents << 1,
			UpdateIndexBuffer = UpdateVertexBuffer << 1,
			UpdateWeightBuffer = UpdateIndexBuffer << 1,
			UpdateAlphaBuffer = UpdateWeightBuffer << 1,
			UpdateVertexAnimationBuffer = UpdateAlphaBuffer << 1,
			UpdateChildren = UpdateVertexAnimationBuffer << 1,

			UpdateBuffers = UpdateVertexBuffer | UpdateIndexBuffer | UpdateWeightBuffer | UpdateAlphaBuffer | UpdateVertexAnimationBuffer,
			Initialize = InitializeCollisionShapes | UpdateBounds | UpdatePrimitiveCounts,
			All = (UpdateChildren << 1) - 1,
			AllData = All & ~UpdateBuffers,
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::asset::ModelUpdateFlags)
};
