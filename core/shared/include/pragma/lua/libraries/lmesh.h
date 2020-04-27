/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LMESH_H__
#define __LMESH_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

namespace Lua
{
	namespace mesh
	{
		DLLNETWORK int generate_convex_hull(lua_State *l);
		DLLNETWORK int calc_smallest_enclosing_bbox(lua_State *l);
	};
};

#endif
