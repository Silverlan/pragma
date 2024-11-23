/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/time.hpp"

using namespace pragma::rendering::shader_graph;

TimeNode::TimeNode(const std::string_view &type) : Node {type}
{
	AddOutput(OUT_TIME, pragma::shadergraph::SocketType::Float);
	AddOutput(OUT_DELTA_TIME, pragma::shadergraph::SocketType::Float);
	AddOutput(OUT_REAL_TIME, pragma::shadergraph::SocketType::Float);
	AddOutput(OUT_DELTA_REAL_TIME, pragma::shadergraph::SocketType::Float);

	AddModuleDependency("time");
}

std::string TimeNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetOutputVarName(OUT_TIME) << " = u_time.time;\n";
	code << instance.GetOutputVarName(OUT_DELTA_TIME) << " = u_time.deltaTime;\n";
	code << instance.GetOutputVarName(OUT_REAL_TIME) << " = u_time.realTime;\n";
	code << instance.GetOutputVarName(OUT_DELTA_REAL_TIME) << " = u_time.deltaRealTime;\n";
	return code.str();
}
