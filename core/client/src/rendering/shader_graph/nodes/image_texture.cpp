/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/image_texture.hpp"

using namespace pragma::rendering::shader_graph;

ImageTextureNode::ImageTextureNode(const std::string_view &type) : ImageTextureNodeBase {type, false}
{
	AddInput(IN_FILENAME, pragma::shadergraph::DataType::String, "");
	AddInput(IN_VECTOR, pragma::shadergraph::DataType::Vector, Vector3 {0.f, 0.f, 0.f}); // TODO: Make input only, don't allow writing manually

	AddOutput(OUT_COLOR, pragma::shadergraph::DataType::Color);
	AddOutput(OUT_ALPHA, pragma::shadergraph::DataType::Float);
}

std::string ImageTextureNode::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	std::string uv;
	if(gn.IsInputLinked(IN_VECTOR))
		uv = gn.GetInputNameOrValue(IN_VECTOR);
	else
		uv = "vec3(get_vertex_uv(), 0.0)";

	auto prefix = gn.GetBaseVarName() + "_";
	code << "vec4 " << prefix << "texCol = texture(" << GetTextureVariableName(gn) << ", " << uv << ".xy);\n";

	code << gn.GetGlslOutputDeclaration(OUT_COLOR) << " = ";
	code << prefix << "texCol.rgb;\n";

	code << gn.GetGlslOutputDeclaration(OUT_ALPHA) << " = ";
	code << prefix << "texCol.a;\n";
	return code.str();
}

//

ImageTextureNodeBase::ImageTextureNodeBase(const std::string_view &type, bool populateOutputsAndInputs) : Node {type, pragma::shadergraph::CATEGORY_TEXTURE}
{
	if(populateOutputsAndInputs) {
		AddInput(IN_FILENAME, pragma::shadergraph::DataType::String, "");

		AddOutput(OUT_TEXTURE, pragma::shadergraph::DataType::String);
	}

	AddModuleDependency("image_texture");
}

std::string ImageTextureNodeBase::GetTextureVariableName(const pragma::shadergraph::OutputSocket &socket) const { return GetTextureVariableName(*socket.parent); }

std::string ImageTextureNodeBase::GetTextureVariableName(const pragma::shadergraph::GraphNode &gn) const
{
	auto prefix = gn.GetBaseVarName() + "_";
	return prefix + "tex";
}

std::string ImageTextureNodeBase::DoEvaluateResourceDeclarations(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	//auto texName = GetTextureVariableName(gn);
	//auto upperTexName = texName;
	//ustring::to_upper(upperTexName);
	//code << "layout(LAYOUT_ID(TEST, " << upperTexName << ")) uniform sampler2D " << texName << ";\n";
	return code.str();
}

std::string ImageTextureNodeBase::DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	return code.str();
}
