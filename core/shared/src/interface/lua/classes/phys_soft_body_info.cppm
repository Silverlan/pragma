// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.phys_soft_body_info;

export import :physics.soft_body_info;
export import pragma.lua;

export namespace Lua {
	namespace PhysSoftBodyInfo {
		DLLNETWORK void register_class(lua::State *l, luabind::class_<pragma::physics::PhysSoftBodyInfo> &classDef);
	};
};
