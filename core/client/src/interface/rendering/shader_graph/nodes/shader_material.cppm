// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:rendering.shader_graph.node_shader_material;

export import :rendering.shader_material;
export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
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
