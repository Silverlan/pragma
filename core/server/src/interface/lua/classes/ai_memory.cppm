// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.server.scripting.lua.classes.ai_memory;

export namespace Lua {
	namespace AIMemory {
		DLLSERVER void register_class(lua_State *l, luabind::module_ &mod);
	};
};
