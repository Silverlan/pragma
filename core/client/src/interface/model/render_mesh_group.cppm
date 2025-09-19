// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client.model:render_mesh_group;

export namespace pragma {
	using RenderMeshIndex = uint32_t;
	using RenderMeshGroup = std::pair<RenderMeshIndex, RenderMeshIndex>; // Start index +count
};
