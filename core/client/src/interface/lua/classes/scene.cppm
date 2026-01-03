// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.bindings.scene;
export import :entities.components;

export namespace pragma::scripting::lua_core::bindings {
	void register_renderers(lua::State *l, luabind::module_ &entsMod);
}
