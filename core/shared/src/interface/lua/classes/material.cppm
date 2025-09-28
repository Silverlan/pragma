// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.shared:scripting.lua.classes.material;

export namespace Lua {
	namespace Material {
		DLLNETWORK void register_class(luabind::class_<::Material> &classDef);
	};
};
