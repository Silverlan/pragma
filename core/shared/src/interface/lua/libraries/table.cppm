// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.table;

export import pragma.lua;

export namespace Lua {
	namespace table {
		DLLNETWORK int has_value(lua::State *l);
		DLLNETWORK int random(lua::State *l);
		DLLNETWORK int is_empty(lua::State *l);
		DLLNETWORK int count(lua::State *l);
	};
};
