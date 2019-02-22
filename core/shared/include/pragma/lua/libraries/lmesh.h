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
