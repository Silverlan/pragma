// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LPHYSOBJ_H__
#define __LPHYSOBJ_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

namespace Lua {
	namespace PhysObj {
		DLLNETWORK void register_class(lua_State *l, luabind::module_ &mod);
	};
};

#endif
