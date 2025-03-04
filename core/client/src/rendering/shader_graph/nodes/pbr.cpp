/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/pbr.hpp"

using namespace pragma::rendering::shader_graph;

PbrNode::PbrNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_SHADER}
{
	AddInput(IN_ALBEDO_COLOR, pragma::shadergraph::DataType::Color, Vector3 {1.f, 1.f, 1.f});
	AddInput(IN_METALNESS, pragma::shadergraph::DataType::Float, 0.f);
	AddInput(IN_ROUGHNESS, pragma::shadergraph::DataType::Float, 0.5f);
	AddInput(IN_AMBIENT_OCCLUSION, pragma::shadergraph::DataType::Float, 0.f);

	AddOutput(OUT_COLOR, pragma::shadergraph::DataType::Color);

	AddModuleDependency("pbr");
}

std::string PbrNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	std::ostringstream code;

	auto inAc = gn.GetInputNameOrValue(IN_ALBEDO_COLOR);
	auto inMetalness = gn.GetInputNameOrValue(IN_METALNESS);
	auto inRoughness = gn.GetInputNameOrValue(IN_ROUGHNESS);
	auto inAo = gn.GetInputNameOrValue(IN_AMBIENT_OCCLUSION);

	auto prefix = gn.GetBaseVarName() + "_";
	auto vA = prefix + "albedo";
	auto vUv = prefix + "uv";
	auto vDbg = prefix + "debugMode";
	auto vMat = prefix + "material";
	auto vEmission = prefix + "emissionFactor";
	auto vMatFlags = prefix + "matFlags";
	code << "vec4 " << vA << " = vec4(" << inAc << ",1);\n";
	code << "vec2 " << vUv << " = get_uv_coordinates();\n";
	code << "uint " << vDbg << " = 0;\n";
	code << "PbrMaterial " << vMat << ";\n";
	code << vMat << ".color = vec4(1,1,1,1);\n";
	code << vMat << ".metalnessFactor = " << inMetalness << ";\n";
	code << vMat << ".roughnessFactor = " << inRoughness << ";\n";
	code << vMat << ".aoFactor = " << inAo << ";\n";
	code << vMat << ".alphaMode = ALPHA_MODE_OPAQUE;\n";
	code << vMat << ".alphaCutoff = 0.5;\n";
	code << "vec3 " << vEmission << " = vec3(0,0,0);\n"; // TODO
	code << "uint " << vMatFlags << " = get_mat_flags();\n";
	code << gn.GetGlslOutputDeclaration(OUT_COLOR) << " = ";
	code << "calc_pbr(" << vA << ", " << vUv << ", " << vDbg << ", " << vMat << ", " << vEmission << ", " << vMatFlags << ").rgb;\n";
	return code.str();
}
