// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.classes.ai_squad;

export import pragma.lua;

export namespace Lua {
	namespace AISquad {
		DLLSERVER void register_class(lua::State *l, luabind::module_ &mod);
	};
};
