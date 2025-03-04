/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/geometry.hpp"

using namespace pragma::rendering::shader_graph;

GeometryNode::GeometryNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_SCENE}
{
	AddOutput(OUT_POSITION_WS, pragma::shadergraph::DataType::Vector);
	AddOutput(OUT_NORMAL_WS, pragma::shadergraph::DataType::Vector);
	AddOutput(OUT_NORMAL_CS, pragma::shadergraph::DataType::Vector);
	AddOutput(OUT_TANGENT_WS, pragma::shadergraph::DataType::Vector);

	AddModuleDependency("vertex_data");
}

std::string GeometryNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
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
