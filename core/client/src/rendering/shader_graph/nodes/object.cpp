/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/object.hpp"

using namespace pragma::rendering::shader_graph;

ObjectNode::ObjectNode(const std::string_view &type) : Node {type}
{
	AddOutput(OUT_MODEL_MATRIX, pragma::shadergraph::SocketType::Transform);
	AddOutput(OUT_COLOR, pragma::shadergraph::SocketType::Color);

	AddModuleDependency("object");
}

std::string ObjectNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetOutputVarName(OUT_MODEL_MATRIX) << " = u_instance.M;\n";
	code << instance.GetOutputVarName(OUT_COLOR) << " = u_instance.color;\n";
	return code.str();
}