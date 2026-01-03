// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.libraries.engine;

export import pragma.materialsystem;

export namespace Lua::engine::server {
	DLLSERVER pragma::material::Material *LoadMaterial(const std::string &mat, bool reload);
	DLLSERVER pragma::material::Material *LoadMaterial(const std::string &mat);
}
