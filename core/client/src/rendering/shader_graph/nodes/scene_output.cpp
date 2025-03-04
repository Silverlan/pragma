/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include <sharedutils/alpha_mode.hpp>
#include "pragma/rendering/shader_graph/nodes/scene_output.hpp"

using namespace pragma::rendering::shader_graph;

SceneOutputNode::SceneOutputNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_OUTPUT}
{
	AddInput(IN_COLOR, pragma::shadergraph::DataType::Color, Vector3 {1.f, 1.f, 1.f});
	AddInput(IN_ALPHA, pragma::shadergraph::DataType::Float, 1.f);
	AddInput(IN_ALPHA_CUTOFF, pragma::shadergraph::DataType::Float, 0.5f);
	AddInput(IN_BLOOM_FACTOR, pragma::shadergraph::DataType::Float, 1.f);
	AddInput(IN_EMISSIVE_COLOR, pragma::shadergraph::DataType::Color, Vector3 {0.f, 0.f, 0.f});

	AddSocketEnum<AlphaMode>(CONST_ALPHA_MODE, AlphaMode::Opaque, true);

	AddModuleDependency("scene_output");
}

std::string SceneOutputNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	code << "fs_color = vec4(" << GetInputNameOrValue(gn, IN_COLOR) << ", " << GetInputNameOrValue(gn, IN_ALPHA) << ");\n";
	// TODO: If no bloom input, calc default bloom
	// TODO: If use_glow_map(materialFlags) == false, we can do optimizations here
	code << "if(CSPEC_BLOOM_OUTPUT_ENABLED == 1)\n";
	code << "{\n";
	auto bloomColor = gn.GetVarName("bloomColor");
	code << "\tvec3 " << bloomColor << " = fs_color.rgb *fs_color.a *" << GetInputNameOrValue(gn, IN_BLOOM_FACTOR) << ";\n";
	code << "\textract_bright_color(vec4(" << bloomColor << ", 1.0), u_renderer.bloomThreshold);\n";
	code << "\tfs_brightColor.rgb += " << GetInputNameOrValue(gn, IN_EMISSIVE_COLOR) << ";\n";
	code << "}\n";

	auto alphaMode = *gn.GetConstantInputValue<AlphaMode>(CONST_ALPHA_MODE);
	if(alphaMode == AlphaMode::Mask) {
		auto alphaCutoff = GetInputNameOrValue(gn, IN_ALPHA_CUTOFF);
		// TODO: Use alpha cutoff value
		code << "if(fs_color.a <= " << alphaCutoff << ")\n";
		code << "\tdiscard;\n";
	}

	return code.str();
}
