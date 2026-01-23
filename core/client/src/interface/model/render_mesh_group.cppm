// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:model.render_mesh_group;

export import std.compat;

export namespace pragma::rendering {
	using RenderMeshIndex = uint32_t;
	using RenderMeshGroup = std::pair<RenderMeshIndex, RenderMeshIndex>; // Start index +count
};
