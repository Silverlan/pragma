// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LMESH_H__
#define __LMESH_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

namespace Lua {
	namespace mesh {
		DLLNETWORK int generate_convex_hull(lua_State *l);
		DLLNETWORK int calc_smallest_enclosing_bbox(lua_State *l);
	};
};

#endif
