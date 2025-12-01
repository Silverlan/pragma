// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.mesh;

export import pragma.lua;

export namespace Lua {
	namespace mesh {
		DLLNETWORK int generate_convex_hull(lua::State *l);
		DLLNETWORK int calc_smallest_enclosing_bbox(lua::State *l);
	};
};
