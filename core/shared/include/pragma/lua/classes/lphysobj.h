/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LPHYSOBJ_H__
#define __LPHYSOBJ_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

namespace Lua
{
	namespace PhysObj
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
};

#endif
