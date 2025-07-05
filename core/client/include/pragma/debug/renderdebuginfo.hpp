// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
