// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.bullet_info;

export import luabind;

export namespace Lua {
	DLLNETWORK void register_bullet_info(luabind::module_ &gameMod);
};
