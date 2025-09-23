// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.shaders.world_water_surface_solve_edges;

import :rendering.shaders.world_water_surface_sum_edges;

export namespace pragma {
	class DLLCLIENT ShaderWaterSurfaceSolveEdges : public ShaderWaterSurfaceSumEdges {
	  public:
		ShaderWaterSurfaceSolveEdges(prosper::IPrContext &context, const std::string &identifier);
	};
};
