// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_fog;

using namespace pragma::rendering::shader_graph;

FogNode::FogNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_ENVIRONMENT}
{
	AddOutput(OUT_COLOR, shadergraph::DataType::Color);
	AddOutput(OUT_START_DISTANCE, shadergraph::DataType::Float);
	AddOutput(OUT_END_DISTANCE, shadergraph::DataType::Float);
	AddOutput(OUT_DENSITY, shadergraph::DataType::Float);

	AddModuleDependency("fog");
}

std::string FogNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetGlslOutputDeclaration(OUT_COLOR) << " = u_fog.color.rgb;\n";
	code << instance.GetGlslOutputDeclaration(OUT_START_DISTANCE) << " = u_fog.start;\n";
	code << instance.GetGlslOutputDeclaration(OUT_END_DISTANCE) << " = u_fog.end;\n";
	code << instance.GetGlslOutputDeclaration(OUT_DENSITY) << " = u_fog.density;\n";
	return code.str();
}
