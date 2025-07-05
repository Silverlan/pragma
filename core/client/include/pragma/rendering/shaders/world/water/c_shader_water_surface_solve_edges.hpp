// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_WATER_SURFACE_SOLVE_EDGES_HPP__
#define __C_SHADER_WATER_SURFACE_SOLVE_EDGES_HPP__

#include "pragma/rendering/shaders/world/water/c_shader_water_surface_sum_edges.hpp"

namespace pragma {
	class DLLCLIENT ShaderWaterSurfaceSolveEdges : public ShaderWaterSurfaceSumEdges {
	  public:
		ShaderWaterSurfaceSolveEdges(prosper::IPrContext &context, const std::string &identifier);
	};
};

#endif
