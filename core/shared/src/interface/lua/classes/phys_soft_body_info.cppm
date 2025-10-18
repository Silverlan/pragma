// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.shared:scripting.lua.classes.phys_soft_body_info;

export import :physics.soft_body_info;

export namespace Lua {
	namespace PhysSoftBodyInfo {
		DLLNETWORK void register_class(lua_State *l, luabind::class_<::PhysSoftBodyInfo> &classDef);
	};
};
