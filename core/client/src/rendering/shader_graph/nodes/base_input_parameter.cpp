/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/base_input_parameter.hpp"

using namespace pragma::rendering::shader_graph;

BaseInputParameterNode::BaseInputParameterNode(const std::string_view &type) : Node {type}
{
	AddInput(CONST_NAME, pragma::shadergraph::DataType::String, "");
	AddSocketEnum<Scope>(CONST_SCOPE, Scope::Global);

	AddModuleDependency("input_data");
}

std::string BaseInputParameterNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
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
