// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LPHYSSOFTBODYINFO_HPP__
#define __LPHYSSOFTBODYINFO_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

struct PhysSoftBodyInfo;
namespace Lua {
	namespace PhysSoftBodyInfo {
		DLLNETWORK void register_class(lua_State *l, luabind::class_<::PhysSoftBodyInfo> &classDef);
	};
};

#endif
