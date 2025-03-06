/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

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
