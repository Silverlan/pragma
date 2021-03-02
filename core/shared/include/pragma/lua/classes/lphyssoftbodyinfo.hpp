/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LPHYSSOFTBODYINFO_HPP__
#define __LPHYSSOFTBODYINFO_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

struct PhysSoftBodyInfo;
namespace Lua
{
	namespace PhysSoftBodyInfo
	{
		DLLNETWORK void register_class(
			lua_State *l,
			luabind::class_<::PhysSoftBodyInfo> &classDef
		);
	};
};

#endif
