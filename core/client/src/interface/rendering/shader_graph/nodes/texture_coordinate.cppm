// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_texture_coordinate;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT TextureCoordinateNode : public shadergraph::Node {
	  public:
		static constexpr const char *OUT_UV = "uv";

		TextureCoordinateNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};
