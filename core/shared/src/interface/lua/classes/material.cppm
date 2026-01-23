// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.material;

export import luabind;
export import pragma.materialsystem;

export namespace Lua {
	namespace Material {
		DLLNETWORK void register_class(luabind::class_<pragma::material::Material> &classDef);
	};
};
