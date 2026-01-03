// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.node_camera;

using namespace pragma::rendering::shader_graph;

CameraNode::CameraNode(const std::string_view &type) : Node {type, shadergraph::CATEGORY_SCENE}
{
	AddOutput(OUT_POSITION, shadergraph::DataType::Vector);
	AddOutput(OUT_FOV, shadergraph::DataType::Float);
	AddOutput(OUT_NEARZ, shadergraph::DataType::Float);
	AddOutput(OUT_FARZ, shadergraph::DataType::Float);
	AddOutput(OUT_VIEW_MATRIX, shadergraph::DataType::Transform);
	AddOutput(OUT_PROJECTION_MATRIX, shadergraph::DataType::Transform);
	AddOutput(OUT_VIEW_PROJECTION_MATRIX, shadergraph::DataType::Transform);

	AddModuleDependency("camera");
	AddModuleDependency("render_settings");
}

std::string CameraNode::DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetGlslOutputDeclaration(OUT_POSITION) << " = u_renderSettings.posCam.xyz;\n";
	code << instance.GetGlslOutputDeclaration(OUT_FOV) << " = u_renderSettings.posCam.w;\n";
	code << instance.GetGlslOutputDeclaration(OUT_NEARZ) << " = u_renderSettings.nearZ;\n";
	code << instance.GetGlslOutputDeclaration(OUT_FARZ) << " = u_renderSettings.farZ;\n";
	code << instance.GetGlslOutputDeclaration(OUT_VIEW_MATRIX) << " = u_camera.V;\n";
	code << instance.GetGlslOutputDeclaration(OUT_PROJECTION_MATRIX) << " = u_camera.P;\n";
	code << instance.GetGlslOutputDeclaration(OUT_VIEW_PROJECTION_MATRIX) << " = u_camera.VP;\n";
	return code.str();
}
