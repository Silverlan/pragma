// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.client;

import :rendering.shader_graph.node_fog;

using namespace pragma::rendering::shader_graph;

FogNode::FogNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_ENVIRONMENT}
{
	AddOutput(OUT_COLOR, pragma::shadergraph::DataType::Color);
	AddOutput(OUT_START_DISTANCE, pragma::shadergraph::DataType::Float);
	AddOutput(OUT_END_DISTANCE, pragma::shadergraph::DataType::Float);
	AddOutput(OUT_DENSITY, pragma::shadergraph::DataType::Float);

	AddModuleDependency("fog");
}

std::string FogNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetGlslOutputDeclaration(OUT_COLOR) << " = u_fog.color.rgb;\n";
	code << instance.GetGlslOutputDeclaration(OUT_START_DISTANCE) << " = u_fog.start;\n";
	code << instance.GetGlslOutputDeclaration(OUT_END_DISTANCE) << " = u_fog.end;\n";
	code << instance.GetGlslOutputDeclaration(OUT_DENSITY) << " = u_fog.density;\n";
	return code.str();
}
