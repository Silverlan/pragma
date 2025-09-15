// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client.rendering.shader_graph;

import :node_time;

using namespace pragma::rendering::shader_graph;

TimeNode::TimeNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_UTILITY}
{
	AddOutput(OUT_TIME, pragma::shadergraph::DataType::Float);
	AddOutput(OUT_DELTA_TIME, pragma::shadergraph::DataType::Float);
	AddOutput(OUT_REAL_TIME, pragma::shadergraph::DataType::Float);
	AddOutput(OUT_DELTA_REAL_TIME, pragma::shadergraph::DataType::Float);

	AddModuleDependency("time");
}

std::string TimeNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetGlslOutputDeclaration(OUT_TIME) << " = cur_time();\n";
	code << instance.GetGlslOutputDeclaration(OUT_DELTA_TIME) << " = delta_time();\n";
	code << instance.GetGlslOutputDeclaration(OUT_REAL_TIME) << " = real_time();\n";
	code << instance.GetGlslOutputDeclaration(OUT_DELTA_REAL_TIME) << " = delta_real_time();\n";
	return code.str();
}
