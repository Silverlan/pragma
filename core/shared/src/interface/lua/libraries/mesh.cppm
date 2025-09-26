// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

export module pragma.client:scripting.lua.libraries.mesh;

export namespace Lua {
	namespace mesh {
		DLLNETWORK int generate_convex_hull(lua_State *l);
		DLLNETWORK int calc_smallest_enclosing_bbox(lua_State *l);
	};
};
