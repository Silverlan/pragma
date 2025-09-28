// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client:rendering.shader_graph.node_texture_coordinate;

import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT TextureCoordinateNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *OUT_UV = "uv";

		TextureCoordinateNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};
