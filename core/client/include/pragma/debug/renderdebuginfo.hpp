/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __RENDERDEBUGINFO_HPP__
#define __RENDERDEBUGINFO_HPP__

#include "pragma/clientdefinitions.h"
#include <mathutil/umath.h>
#include <chrono>
#include <memory>

class DLLCLIENT RenderDebugInfo {
  public:
	uint32_t triangleCount = 0;
	uint32_t vertexCount = 0;
	uint32_t shadowMeshCount = 0;
	uint32_t meshCount = 0;
	uint32_t staticMeshCount = 0;
	uint32_t materialCount = 0;
	uint32_t shadowEntityCount = 0;
	uint32_t entityCount = 0;
	uint32_t shaderCount = 0;
	void Reset();
};

RenderDebugInfo &get_render_debug_info();

#endif
