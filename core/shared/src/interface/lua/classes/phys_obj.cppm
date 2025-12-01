// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.phys_obj;

export import pragma.lua;

export namespace Lua {
	namespace PhysObj {
		DLLNETWORK void register_class(lua::State *l, luabind::module_ &mod);
	};
};
