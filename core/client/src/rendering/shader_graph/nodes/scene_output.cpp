/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/scene_output.hpp"

using namespace pragma::rendering::shader_graph;

SceneOutputNode::SceneOutputNode(const std::string_view &type) : Node {type}
{
	AddInput(IN_COLOR, pragma::shadergraph::SocketType::Color, Vector3 {1.f, 1.f, 1.f});
	AddInput(IN_ALPHA, pragma::shadergraph::SocketType::Float, 1.f);
	AddInput(IN_BLOOM_COLOR, pragma::shadergraph::SocketType::Color, Vector3 {0.f, 0.f, 0.f});
}

std::string SceneOutputNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << "fs_color = vec4(" << GetInputNameOrValue(instance, IN_COLOR) << ", " << GetInputNameOrValue(instance, IN_ALPHA) << ");\n";
	code << "fs_brightColor = vec4(" << GetInputNameOrValue(instance, IN_BLOOM_COLOR) << ", 1.0);\n";
	return code.str();
	//
}
