/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/camera.hpp"

using namespace pragma::rendering::shader_graph;

CameraNode::CameraNode(const std::string_view &type) : Node {type}
{
	AddOutput(OUT_POSITION, pragma::shadergraph::SocketType::Vector);
	AddOutput(OUT_FOV, pragma::shadergraph::SocketType::Float);
	AddOutput(OUT_NEARZ, pragma::shadergraph::SocketType::Float);
	AddOutput(OUT_FARZ, pragma::shadergraph::SocketType::Float);
	AddOutput(OUT_VIEW_MATRIX, pragma::shadergraph::SocketType::Transform);
	AddOutput(OUT_PROJECTION_MATRIX, pragma::shadergraph::SocketType::Transform);
	AddOutput(OUT_VIEW_PROJECTION_MATRIX, pragma::shadergraph::SocketType::Transform);

	AddModuleDependency("camera");
	AddModuleDependency("render_settings");
}

std::string CameraNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const
{
	std::ostringstream code;
	code << instance.GetOutputVarName(OUT_POSITION) << " = u_renderSettings.posCam.xyz;\n";
	code << instance.GetOutputVarName(OUT_FOV) << " = u_renderSettings.posCam.w;\n";
	code << instance.GetOutputVarName(OUT_NEARZ) << " = u_renderSettings.nearZ;\n";
	code << instance.GetOutputVarName(OUT_FARZ) << " = u_renderSettings.farZ;\n";
	code << instance.GetOutputVarName(OUT_VIEW_MATRIX) << " = u_camera.V;\n";
	code << instance.GetOutputVarName(OUT_PROJECTION_MATRIX) << " = u_camera.P;\n";
	code << instance.GetOutputVarName(OUT_VIEW_PROJECTION_MATRIX) << " = u_camera.VP;\n";
	return code.str();
}
