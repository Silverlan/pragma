/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/toon.hpp"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include "pragma/rendering/shader_graph/nodes/image_texture.hpp"
#include "pragma/rendering/shader_graph/nodes/shader_material.hpp"

using namespace pragma::rendering::shader_graph;
ToonNode::ToonNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_SHADER}
{
	AddInput(IN_ALBEDO_COLOR, pragma::shadergraph::DataType::Color, Vector3 {1.f, 1.f, 1.f});
	AddInput(IN_ALPHA, pragma::shadergraph::DataType::Float, 1.f);
	AddInput(IN_SPECULAR_COLOR, pragma::shadergraph::DataType::Color, Vector3 {1.f, 1.f, 1.f});
	AddInput(IN_GLOSSINESS, pragma::shadergraph::DataType::Float, 1.f);
	AddInput(IN_RIM_AMOUNT, pragma::shadergraph::DataType::Float, 0.5f);
	AddInput(IN_RIM_COLOR, pragma::shadergraph::DataType::Color, Vector3 {1.f, 1.f, 1.f});
	AddInput(IN_MATCAP_TEXTURE, pragma::shadergraph::DataType::String, "");
	AddInput(IN_RAMP_TEXTURE, pragma::shadergraph::DataType::String, "");

	AddOutput(OUT_COLOR, pragma::shadergraph::DataType::Color);

	AddModuleDependency("toon");
}

static std::optional<std::string> get_image_texture_variable_name(const pragma::shadergraph::InputSocket &input)
{
	auto *texNode = dynamic_cast<const ImageTextureNodeBase *>(&input.link->parent->node);
	if(texNode)
		return texNode->GetTextureVariableName(*input.link);
	auto *matNode = dynamic_cast<const ShaderMaterialNode *>(&input.link->parent->node);
	if(matNode)
		return matNode->GetTextureVariableName(*input.link);
	return {};
}

std::string ToonNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	auto inAlbedoColor = gn.GetInputNameOrValue(IN_ALBEDO_COLOR);
	auto inAlpha = gn.GetInputNameOrValue(IN_ALPHA);
	auto inSpecularColor = gn.GetInputNameOrValue(IN_SPECULAR_COLOR);
	auto inGlossiness = gn.GetInputNameOrValue(IN_GLOSSINESS);
	auto inRimAmount = gn.GetInputNameOrValue(IN_RIM_AMOUNT);
	auto inRimColor = gn.GetInputNameOrValue(IN_RIM_COLOR);

	auto prefix = gn.GetBaseVarName() + "_";
	auto vMatFlags = prefix + "matFlags";
	auto vToonCol = prefix + "toonCol";
	auto vMatcapCol = prefix + "matcapCol";

	std::ostringstream code;
	code << gn.GetGlslOutputDeclaration(OUT_COLOR) << ";\n";
	auto outCol = gn.GetOutputVarName(OUT_COLOR);
	code << "uint " << vMatFlags << " = get_mat_flags();\n";
	code << "vec3 " << vToonCol << " = calc_toon(";
	code << "vec4(" << inAlbedoColor << ", " << inAlpha << "), " << inSpecularColor << ", " << inGlossiness << ", " << inRimAmount << ", ";
	code << inRimColor << ", " << vMatFlags << ", ";

	auto *matcapInputSocket = gn.FindInputSocket(IN_MATCAP_TEXTURE);
	if(matcapInputSocket && matcapInputSocket->link) {
		auto texVarName = get_image_texture_variable_name(*matcapInputSocket);
		if(texVarName)
			code << *texVarName;
	}
	code << ", ";

	auto *rampInputSocket = gn.FindInputSocket(IN_RAMP_TEXTURE);
	auto *texNode = dynamic_cast<const ImageTextureNodeBase *>(&rampInputSocket->link->parent->node);
	if(rampInputSocket && rampInputSocket->link) {
		auto texVarName = get_image_texture_variable_name(*rampInputSocket);
		if(texVarName)
			code << *texVarName;
	}

	code << ").rgb;\n";

	code << outCol << " = " << vToonCol << ".rgb;\n";
	return code.str();
}
