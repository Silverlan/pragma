// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.node_toon;

export import pragma.shadergraph;

export namespace pragma::rendering::shader_graph {
	class DLLCLIENT ToonNode : public shadergraph::Node {
	  public:
		static constexpr const char *IN_ALBEDO_COLOR = "albedoColor";
		static constexpr const char *IN_ALPHA = "alpha";
		static constexpr const char *IN_SPECULAR_COLOR = "specularColor";
		static constexpr const char *IN_GLOSSINESS = "glossiness";
		static constexpr const char *IN_RIM_AMOUNT = "rimAmount";
		static constexpr const char *IN_RIM_COLOR = "rimColor";
		static constexpr const char *IN_MATCAP_TEXTURE = "matcapTexture";
		static constexpr const char *IN_RAMP_TEXTURE = "rampTexture";

		static constexpr const char *OUT_COLOR = "color";

		ToonNode(const std::string_view &type);

		virtual std::string DoEvaluate(const shadergraph::Graph &graph, const shadergraph::GraphNode &instance) const override;
	};
};
