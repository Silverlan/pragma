// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __MODELUPDATEFLAGS_HPP__
#define __MODELUPDATEFLAGS_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class DLLNETWORK ModelUpdateFlags : uint32_t {
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
REGISTER_BASIC_BITWISE_OPERATORS(ModelUpdateFlags);

#endif
