// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.shared:scripting.lua.classes.phys_obj;

export namespace Lua {
	namespace pragma::physics::PhysObj {
		DLLNETWORK void register_class(lua_State *l, luabind::module_ &mod);
	};
};
