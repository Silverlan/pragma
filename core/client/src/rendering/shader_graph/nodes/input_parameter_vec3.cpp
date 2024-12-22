/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/input_parameter_vec3.hpp"

using namespace pragma::rendering::shader_graph;

InputParameterVec3Node::InputParameterVec3Node(const std::string_view &type) : BaseInputParameterNode {type}
{
	AddInput(CONST_DEFAULT, pragma::shadergraph::DataType::Vector, Vector3 {0.f, 0.f, 0.f});

	AddOutput(OUT_VALUE, pragma::shadergraph::DataType::Vector);
}

std::string InputParameterVec3Node::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	std::ostringstream code;

	std::string name;
	gn.GetInputValue<std::string>(CONST_NAME, name);

	code << gn.GetGlslOutputDeclaration(OUT_VALUE) << " = ";
	// TODO: Check if name exists in global input data
	if(!name.empty())
		code << "u_globalInputData." << name << ";\n";
	else
		code << "0.0;\n";

	return code.str();
}
