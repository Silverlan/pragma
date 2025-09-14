// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_RENDER_MESH_GROUP_HPP__
#define __PRAGMA_RENDER_MESH_GROUP_HPP__

#include "pragma/clientdefinitions.h"

namespace pragma {
	using RenderMeshIndex = uint32_t;
	using RenderMeshGroup = std::pair<RenderMeshIndex, RenderMeshIndex>; // Start index +count
};

#endif
