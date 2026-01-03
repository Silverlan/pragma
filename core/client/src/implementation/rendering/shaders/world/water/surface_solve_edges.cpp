// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_water_surface_solve_edges;

using namespace pragma;

ShaderWaterSurfaceSolveEdges::ShaderWaterSurfaceSolveEdges(prosper::IPrContext &context, const std::string &identifier) : ShaderWaterSurfaceSumEdges(context, identifier, "programs/compute/water/water_surface_solve_edges") {}
