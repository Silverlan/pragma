/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/material_texture.hpp"
#include "pragma/rendering/shader_graph/nodes/input_parameter.hpp"

using namespace pragma::rendering::shader_graph;
MaterialTextureNode::MaterialTextureNode(const std::string_view &type) : Node {type, pragma::shadergraph::CATEGORY_TEXTURE}
{
	AddInput(IN_TEXTURE, pragma::shadergraph::DataType::String, "");
	AddInput(IN_VECTOR, pragma::shadergraph::DataType::Vector, Vector3 {0.f, 0.f, 0.f}); // TODO: Make input only, don't allow writing manually

	AddOutput(OUT_COLOR, pragma::shadergraph::DataType::Color);
	AddOutput(OUT_ALPHA, pragma::shadergraph::DataType::Float);
}

std::string MaterialTextureNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	std::string uv;
	if(gn.IsInputLinked(IN_VECTOR))
		uv = gn.GetInputNameOrValue(IN_VECTOR);
	else
		uv = "vec3(get_vertex_uv(), 0.0)";

	auto prefix = gn.GetBaseVarName() + "_";
	auto *texInputSocket = gn.FindInputSocket(IN_TEXTURE);
	if(texInputSocket && texInputSocket->link) {
		std::string texName = ustring::to_snake_case(texInputSocket->link->GetSocket().name);
		if(texInputSocket->link->parent) {
			auto *dynTexNode = dynamic_cast<const pragma::rendering::shader_graph::InputParameterTextureNode *>(&texInputSocket->link->parent->node);
			if(dynTexNode) {
				std::string name;
				if(texInputSocket->link->parent->GetInputValue(pragma::rendering::shader_graph::BaseInputParameterNode::CONST_NAME, name))
					texName = name;
			}
		}
		code << "vec4 " << prefix << "texCol = fetch_" << texName << "(" << uv << ".xy);\n";
	}
	else
		code << "vec4 " << prefix << "texCol = vec4(1.0, 1.0, 1.0, 1.0);\n";

	code << gn.GetGlslOutputDeclaration(OUT_COLOR) << " = ";
	code << prefix << "texCol.rgb;\n";

	code << gn.GetGlslOutputDeclaration(OUT_ALPHA) << " = ";
	code << prefix << "texCol.a;\n";
	return code.str();
}
