// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_shader_material;

export import :rendering.shader_material;
export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT ShaderMaterialNode : public shadergraph::Node {
	  public:
		ShaderMaterialNode(const std::string_view &type, const shader_material::ShaderMaterial &shaderMaterial);

		std::string GetTextureVariableName(const shadergraph::OutputSocket &socket) const;

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
		const shader_material::ShaderMaterial &GetShaderMaterial() const { return m_shaderMaterial; }
	  private:
		const shader_material::ShaderMaterial &m_shaderMaterial;
	};
};
