// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.server:scripting.lua.libraries.engine;

export import pragma.materialsystem;

export namespace Lua::engine::server {
	DLLSERVER msys::Material *LoadMaterial(const std::string &mat, bool reload);
	DLLSERVER msys::Material *LoadMaterial(const std::string &mat);
}
