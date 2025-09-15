// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client.rendering.shader_graph;

import :node_object;

using namespace pragma::rendering::shader_graph;

ObjectNode::ObjectNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_SCENE}
{
	AddOutput(OUT_MODEL_MATRIX, pragma::shadergraph::DataType::Transform);
	AddOutput(OUT_COLOR, pragma::shadergraph::DataType::Color);

	AddModuleDependency("object");
}

std::string ObjectNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetGlslOutputDeclaration(OUT_MODEL_MATRIX) << " = u_instance.M;\n";
	code << instance.GetGlslOutputDeclaration(OUT_COLOR) << " = u_instance.color;\n";
	return code.str();
}
