/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_WATER_SURFACE_SOLVE_EDGES_HPP__
#define __C_SHADER_WATER_SURFACE_SOLVE_EDGES_HPP__

#include "pragma/rendering/shaders/world/water/c_shader_water_surface_sum_edges.hpp"

namespace pragma
{
	class DLLCLIENT ShaderWaterSurfaceSolveEdges
		: public ShaderWaterSurfaceSumEdges
	{
	public:
		ShaderWaterSurfaceSolveEdges(prosper::IPrContext &context,const std::string &identifier);
	};
};

#endif
