// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_scene_output;

using namespace pragma::rendering::shader_graph;

SceneOutputNode::SceneOutputNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_OUTPUT}
{
	AddInput(IN_COLOR, shadergraph::DataType::Color, Vector3 {1.f, 1.f, 1.f});
	AddInput(IN_ALPHA, shadergraph::DataType::Float, 1.f);
	AddInput(IN_ALPHA_CUTOFF, shadergraph::DataType::Float, 0.5f);
	AddInput(IN_BLOOM_FACTOR, shadergraph::DataType::Float, 1.f);
	AddInput(IN_EMISSIVE_COLOR, shadergraph::DataType::Color, Vector3 {0.f, 0.f, 0.f});

	AddSocketEnum<AlphaMode>(CONST_ALPHA_MODE, AlphaMode::Opaque, true);

	AddModuleDependency("scene_output");
}

std::string SceneOutputNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &gn) const
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
