// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_geometry;

using namespace pragma::rendering::shader_graph;

GeometryNode::GeometryNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_SCENE}
{
	AddOutput(OUT_POSITION_WS, shadergraph::DataType::Vector);
	AddOutput(OUT_NORMAL_WS, shadergraph::DataType::Vector);
	AddOutput(OUT_NORMAL_CS, shadergraph::DataType::Vector);
	AddOutput(OUT_TANGENT_WS, shadergraph::DataType::Vector);

	AddModuleDependency("vertex_data");
}

std::string GeometryNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	code << gn.GetGlslOutputDeclaration(OUT_POSITION_WS) << " = get_vertex_position_ws();\n";
	code << gn.GetGlslOutputDeclaration(OUT_NORMAL_WS) << " = get_vertex_normal();\n";
	code << gn.GetGlslOutputDeclaration(OUT_NORMAL_CS) << " = get_vertex_normal_cs();\n";

	auto prefix = gn.GetBaseVarName() + "_";
	std::string tbn = prefix + "tbn";
	code << "mat3 " << tbn << " = get_tbn_matrix();\n";
	code << gn.GetGlslOutputDeclaration(OUT_TANGENT_WS) << " = normalize(" << tbn << "[0]);\n";
	return code.str();
}
