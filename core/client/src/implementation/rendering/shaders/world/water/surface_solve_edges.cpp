// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_command_buffer.hpp>

module pragma.client;


import :rendering.shaders.world_water_surface_solve_edges;

using namespace pragma;

ShaderWaterSurfaceSolveEdges::ShaderWaterSurfaceSolveEdges(prosper::IPrContext &context, const std::string &identifier) : ShaderWaterSurfaceSumEdges(context, identifier, "programs/compute/water/water_surface_solve_edges") {}
