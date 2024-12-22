/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/nodes/image_texture.hpp"

using namespace pragma::rendering::shader_graph;

ImageTextureNode::ImageTextureNode(const std::string_view &type) : Node {type}
{
	AddInput(IN_FILENAME, pragma::shadergraph::DataType::String, "");
	AddInput(IN_VECTOR, pragma::shadergraph::DataType::Vector, Vector3 {0.f, 0.f, 0.f}); // TODO: Make input only, don't allow writing manually

	AddOutput(OUT_COLOR, pragma::shadergraph::DataType::Color);
	AddOutput(OUT_ALPHA, pragma::shadergraph::DataType::Float);

	AddModuleDependency("image_texture");
}

std::string ImageTextureNode::DoEvaluateResourceDeclarations(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &gn) const
{
	std::ostringstream code;
	auto prefix = gn.GetBaseVarName() + "_";
	std::string texName = prefix + "tex";
	auto upperTexName = texName;
	ustring::to_upper(upperTexName);
	code << "layout(LAYOUT_ID(TEST, " << upperTexName << ")) uniform sampler2D " << texName << ";\n";
	return code.str();
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
	std::string texName = prefix + "tex";
	code << "vec4 " << prefix << "texCol = texture(" << texName << ", " << uv << ".xy);\n";

	code << gn.GetGlslOutputDeclaration(OUT_COLOR) << " = ";
	code << prefix << "texCol.rgb;\n";

	code << gn.GetGlslOutputDeclaration(OUT_ALPHA) << " = ";
	code << prefix << "texCol.a;\n";
	return code.str();
}
