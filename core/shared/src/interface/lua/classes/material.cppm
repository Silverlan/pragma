// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"

export module pragma.shared:scripting.lua.classes.material;

export namespace Lua {
	namespace msys::Material {
		DLLNETWORK void register_class(luabind::class_<msys::Material> &classDef);
	};
};
