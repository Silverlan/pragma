/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __MODELUPDATEFLAGS_HPP__
#define __MODELUPDATEFLAGS_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class DLLNETWORK ModelUpdateFlags : uint32_t {
	None = 0,
	UpdateBounds = 1,
	UpdatePrimitiveCounts = UpdateBounds << 1,
	UpdateCollisionShapes = UpdatePrimitiveCounts << 1,
	UpdateTangents = UpdateCollisionShapes << 1,
	UpdateVertexBuffer = UpdateTangents << 1,
	UpdateIndexBuffer = UpdateVertexBuffer << 1,
	UpdateWeightBuffer = UpdateIndexBuffer << 1,
	UpdateAlphaBuffer = UpdateWeightBuffer << 1,
	UpdateVertexAnimationBuffer = UpdateAlphaBuffer << 1,
	UpdateChildren = UpdateVertexAnimationBuffer << 1,

	UpdateBuffers = UpdateVertexBuffer | UpdateIndexBuffer | UpdateWeightBuffer | UpdateAlphaBuffer | UpdateVertexAnimationBuffer,
	All = (UpdateChildren << 1) - 1,
	AllData = All & ~UpdateBuffers
};
REGISTER_BASIC_BITWISE_OPERATORS(ModelUpdateFlags);

#endif
