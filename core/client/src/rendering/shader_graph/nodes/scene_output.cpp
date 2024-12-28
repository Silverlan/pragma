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
	AddInput(IN_COLOR, pragma::shadergraph::DataType::Color, Vector3 {1.f, 1.f, 1.f});
	AddInput(IN_ALPHA, pragma::shadergraph::DataType::Float, 1.f);
	AddInput(IN_BLOOM_COLOR, pragma::shadergraph::DataType::Color, Vector3 {0.f, 0.f, 0.f});

	AddSocketEnum<AlphaMode>(CONST_ALPHA_MODE, AlphaMode::Opaque);
}

std::string SceneOutputNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	code << "fs_color = vec4(" << GetInputNameOrValue(gn, IN_COLOR) << ", " << GetInputNameOrValue(gn, IN_ALPHA) << ");\n";
	code << "fs_brightColor = vec4(" << GetInputNameOrValue(gn, IN_BLOOM_COLOR) << ", 1.0);\n";

	auto alphaMode = *gn.GetConstantInputValue<AlphaMode>(CONST_ALPHA_MODE);
	if(alphaMode == AlphaMode::Mask) {
		// TODO: Use alpha cutoff value
		code << "if(fs_color.a < 0.5)\n";
		code << "\tdiscard;\n";
	}

	return code.str();
}
