// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_texture_coordinate;

using namespace pragma::rendering::shader_graph;

TextureCoordinateNode::TextureCoordinateNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_TEXTURE}
{
	AddOutput(OUT_UV, shadergraph::DataType::Vector);

	AddModuleDependency("uv_data");
}

std::string TextureCoordinateNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetGlslOutputDeclaration(OUT_UV) << " = vec3(get_uv_coordinates(), 0.0);\n";
	return code.str();
}
