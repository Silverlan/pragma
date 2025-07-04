// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_SHADER_GRAPH_NODES_SHADER_MATERIAL_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_SHADER_MATERIAL_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shader_material/shader_material.hpp"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT ShaderMaterialNode : public pragma::shadergraph::Node {
	  public:
		ShaderMaterialNode(const std::string_view &type, const pragma::rendering::shader_material::ShaderMaterial &shaderMaterial);

		std::string GetTextureVariableName(const pragma::shadergraph::OutputSocket &socket) const;

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
		const pragma::rendering::shader_material::ShaderMaterial &GetShaderMaterial() const { return m_shaderMaterial; }
	  private:
		const pragma::rendering::shader_material::ShaderMaterial &m_shaderMaterial;
	};
};

#endif
