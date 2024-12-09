/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_SHADER_GRAPH_NODES_CAMERA_HPP__
#define __PRAGMA_SHADER_GRAPH_NODES_CAMERA_HPP__

#include "pragma/clientdefinitions.h"

import pragma.shadergraph;

namespace pragma::rendering::shader_graph {
	class DLLCLIENT CameraNode : public pragma::shadergraph::Node {
	  public:
		static constexpr const char *OUT_POSITION = "position";
		static constexpr const char *OUT_FOV = "fov";
		static constexpr const char *OUT_NEARZ = "nearZ";
		static constexpr const char *OUT_FARZ = "farZ";
		static constexpr const char *OUT_VIEW_MATRIX = "viewMatrix";
		static constexpr const char *OUT_PROJECTION_MATRIX = "projectionMatrix";
		static constexpr const char *OUT_VIEW_PROJECTION_MATRIX = "viewProjectionMatrix";

		CameraNode(const std::string_view &type);

		virtual std::string DoEvaluate(const pragma::shadergraph::Graph &graph, const pragma::shadergraph::GraphNode &instance) const override;
	};
};

#if 0
layout(std140, LAYOUT_ID(SCENE, RENDER_SETTINGS)) uniform RenderSettings
{
	vec4 posCam; // w component is fov
	int flags;
	float shadowRatioX;
	float shadowRatioY;
	float nearZ;
	float farZ;
	int shaderQuality; // 1 = lowest, 10 = highest
}
u_renderSettings;
#endif

#endif
