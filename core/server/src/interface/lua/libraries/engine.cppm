// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "material.h"

export module pragma.server.scripting.lua.libraries.engine;

export namespace Lua::engine::server {
	DLLSERVER Material *LoadMaterial(const std::string &mat, bool reload);
	DLLSERVER Material *LoadMaterial(const std::string &mat);
}
