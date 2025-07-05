// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/texture_coordinate.hpp"

using namespace pragma::rendering::shader_graph;

TextureCoordinateNode::TextureCoordinateNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_TEXTURE}
{
	AddOutput(OUT_UV, pragma::shadergraph::DataType::Vector);

	AddModuleDependency("uv_data");
}

std::string TextureCoordinateNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetGlslOutputDeclaration(OUT_UV) << " = vec3(get_uv_coordinates(), 0.0);\n";
	return code.str();
}
