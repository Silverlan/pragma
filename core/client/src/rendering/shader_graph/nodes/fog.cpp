/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/fog.hpp"

using namespace pragma::rendering::shader_graph;

FogNode::FogNode(const std::string_view &type) : Node {type}
{
	AddOutput(OUT_COLOR, pragma::shadergraph::SocketType::Color);
	AddOutput(OUT_START_DISTANCE, pragma::shadergraph::SocketType::Float);
	AddOutput(OUT_END_DISTANCE, pragma::shadergraph::SocketType::Float);
	AddOutput(OUT_DENSITY, pragma::shadergraph::SocketType::Float);

	AddModuleDependency("fog");
}

std::string FogNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetOutputVarName(OUT_COLOR) << " = u_fog.color.rgb;\n";
	code << instance.GetOutputVarName(OUT_START_DISTANCE) << " = u_fog.start;\n";
	code << instance.GetOutputVarName(OUT_END_DISTANCE) << " = u_fog.end;\n";
	code << instance.GetOutputVarName(OUT_DENSITY) << " = u_fog.density;\n";
	return code.str();
}