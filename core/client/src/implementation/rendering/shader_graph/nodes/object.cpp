// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_object;

using namespace pragma::rendering::shader_graph;

ObjectNode::ObjectNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_SCENE}
{
	AddOutput(OUT_MODEL_MATRIX, shadergraph::DataType::Transform);
	AddOutput(OUT_COLOR, shadergraph::DataType::Color);

	AddModuleDependency("object");
}

std::string ObjectNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetGlslOutputDeclaration(OUT_MODEL_MATRIX) << " = u_instance.M;\n";
	code << instance.GetGlslOutputDeclaration(OUT_COLOR) << " = u_instance.color;\n";
	return code.str();
}
